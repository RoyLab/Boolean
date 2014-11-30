//
//   $Id: //NavisWorks/main/src/lib/lcldfbx/LiFBXNETApi.cpp#4 $
//   $Change: 107559 $
//   $Date: 2013/12/11 $
//
//   Description:
//      Fbx .NET functions
//
//
//   Copyright Notice:
//
//     Copyright 2009 Autodesk, Inc.  All rights reserved.
//  
//     This computer source code and related instructions and 
//     comments are the unpublished confidential and proprietary
//     information of Autodesk, Inc. and are protected under 
//     United States and foreign intellectual property laws.
//     They may not be disclosed to, copied or used by any third
//     party without the prior written consent of Autodesk, Inc.
//
#include "precompiled.h"
#include "LiFBXNETApi.h"
#include "lcutil/LcUString.h"
#include <math.h>

using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;

bool 
LiFBXConvert2JPG(LcUWideStringBuffer& tiff)
{
   try
   {
      System::String^ str = gcnew System::String((WCHAR*)tiff.GetString());
      Image^ file = Image::FromFile(str, FALSE);
      if(file->RawFormat != ImageFormat::Jpeg)
      {
         str = str + L".jpg";
         tiff.Append(L".jpg");
         file->Save(str, ImageFormat::Jpeg);
      }
   }
   catch (Exception^)
   {
      return false;
   }

   return true;
}

// Although some images have alpha channel,
// all the alpha value is 255, so it is totally opaque.
// In theory, it should not matter.
// However, protein an issue of the cutout/blend, so if the cutout is not really needed, we should not use it.
// Explanation of the issue from Protein team
/*
"Transparency handling involves disabling depth testing and sorting objects back to front.  Triangles are not sorted, so you can easily get artifacts within objects.  With high opacity (in this case, full opacity), the artifacts within objects are very visible." - Mauricio Vives <mauricio.vives@autodesk.com>
*/

// HasTransparency will run per-pixel test to see if it is really has transparency
// The way it works: test every pixel, if more than 99% pixels are opaque (alpha value > 250), return false
#define TRANSPARENCYRATIO 0.01
#define ALPHATHRES 250

// per-pixel test, to see if it really has transparency
bool HasTransparency(Bitmap^ img)
{
   if(!img)
      return false;

   bool ret = false;
   try{
      PixelFormat pf = img->PixelFormat;

      if(pf!=PixelFormat::Format32bppArgb && pf!=PixelFormat::Format32bppPArgb)
         return true;

      //lock the bitmap in memory
      BitmapData^ data = img->LockBits(
         System::Drawing::Rectangle(0, 0, img->Width, img->Height),
         ImageLockMode::ReadOnly, pf);

      if(!data)
         return false;

      long pixel_num = img->Width*img->Height;
      // tranparent pixel number threshold
      long pixel_num_thre = long(ceil(pixel_num*TRANSPARENCYRATIO));
      long transparent_pixels = 0;
      int pixel_size = 4; //Format32bppArgb  Format32bppPArgb

      for (int y = 0; y < img->Height; y++)
      {
         unsigned char* row = (unsigned char*)((data->Scan0).ToPointer()) + (y * data->Stride);

         for (int x = 0; x < img->Width; x++)
         {
            unsigned char* color = (row+x * pixel_size);

            // alpha is the last one in color
            if(color[3]<=ALPHATHRES)
            {
               if(++transparent_pixels > pixel_num_thre)
               {
                  ret=true;
                  break;
               }
            }
         }

         if(transparent_pixels > pixel_num_thre)
         {
            ret=true;
            break;
         }
      }

      img->UnlockBits(data);
   }
   catch(Exception^)
   {
      return false;
   }

   return ret;
}


// @Param op_mask, caller have to delete the output file.

// get the opacity mask (grayscale) from the texture map which may have alpha channel
// FBX can use the alpha channel of the texture map to set the transparency of the model,
// but Protein can't do this, it only can use Black/White as alpha source
// this method will extract the alpha channel from the source file, and export the alpha channel to grayscale
bool LiFBXTryGetOpacityMask(const LtWChar* src_file, LcUWideString* op_mask)
{
   if(!op_mask || LcUWideString::IsEmpty(src_file))
      return false;

   System::String^ fn = gcnew System::String(src_file);
   try
   {
      Bitmap^ src = gcnew Bitmap(fn);
      if(src)
      {
         PixelFormat pf = src->PixelFormat;
         bool has_alpha = (pf & PixelFormat::Alpha) == PixelFormat::Alpha;
         
         // to check if the source image contains alpha values that are not premultiplied
         if(!has_alpha)
            return false;

         // see the comment of HasTransparency
         if(!HasTransparency(src))
            return false;

         // Create new bmp which will be the opacity mask
         // first set bg color to white, then use color matrix to set fg color to black
         // when draw image, the alpha will used to blend the fg and bg color,
         // in order to convert alpha to grayscale
         // finally white for transparency, black for opacity
         Bitmap^ bmp = gcnew Bitmap(src->Width, src->Height, PixelFormat::Format24bppRgb);
         if(bmp)
         {
            Graphics^ g = Graphics::FromImage(bmp);
            if(!g)
               return false;

            g->SmoothingMode = Drawing2D::SmoothingMode::HighQuality;
            g->Clear(Color::White);

            // use color matrix to set fg color to black
            ColorMatrix^ colorMatrix = gcnew ColorMatrix(
               gcnew array<array<float,1>^,1>
               {
                  gcnew array<float, 1> {0, 0, 0, 0, 0},
                  gcnew array<float, 1> {0, 0, 0, 0, 0},
                  gcnew array<float, 1> {0, 0, 0, 0, 0},
                  gcnew array<float, 1> {0, 0, 0, 1, 0},
                  gcnew array<float, 1> {0, 0, 0, 0, 1}
               });
            ImageAttributes^ attributes = gcnew ImageAttributes();
            attributes->SetColorMatrix(colorMatrix);

            //draw the original image on the new image
            g->DrawImage(src, System::Drawing::Rectangle(0, 0, src->Width, src->Height),
               0, 0, src->Width, src->Height, GraphicsUnit::Pixel, attributes);

            // Get a temp file name
            LcUWideStringBuffer dir;
            LcUSysFile::MakeTempFileW(NULL, L"Navisworks", L"jpg", 0, &dir);
            if(!dir.IsEmpty())
            {
               bmp->Save(gcnew System::String(dir.GetString()), ImageFormat::Jpeg);

               *op_mask = dir.GetString();

               return true;
            }
         }
      }
   }
   catch(Exception^)
   {
      return false;
   }

   return false;
}
