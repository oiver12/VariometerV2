using System;
using System.Drawing;
using System.IO;
using System.Text;

namespace WindowsFormsApp1
{
    class ImageToBitMapBuffer
    {
        public static void saveBufferImage(String path)
        {
            Bitmap img = new Bitmap(path);
            int widthInBytes = img.Width % 8 == 0 ? img.Width / 8 : img.Width / 8 + 1;
            Byte[] buffer = new Byte[widthInBytes * img.Height];
            for (int x = 0; x < img.Width; x++)
            {
                for (int y = 0; y < img.Height; y++)
                {
                    Color pixel = img.GetPixel(x, y);
                    ulong argbValue = (ulong)pixel.R << 16 | (ulong)pixel.G << 8 | (ulong)pixel.B;
                    string hexValue = argbValue.ToString("X");
                    if(argbValue < 8388608) //Black
                        buffer[x/8 + y*widthInBytes] = (Byte)(buffer[x / 8 + y * widthInBytes] & ~(0x80 >> (x % 8)));
                    else
                        buffer[x/8+y*widthInBytes] = (Byte)(buffer[x / 8 + y * widthInBytes] | (0x80 >> (x % 8)));
                }
            }
            using (FileStream outputFile = File.Open("../../OutputFileBuffer.txt", FileMode.Open, FileAccess.Write))
            {
                outputFile.SetLength(0);
                string tempString = "const unsigned char gImage_Temp[";
                tempString += buffer.Length;
                tempString += "] = {";
                byte[] tempInfo = new UTF8Encoding(true).GetBytes(tempString);
                outputFile.Write(tempInfo, 0, tempInfo.Length);
                for (int i = 0; i < buffer.Length; i++)
                {
                    tempString = $"0x{buffer[i]:X}";
                    if (i < buffer.Length - 1)
                        tempString += ", ";
                    byte[] info = new UTF8Encoding(true).GetBytes(tempString);
                    outputFile.Write(info, 0, info.Length);
                }
                tempString = "};";
                byte[] test2 = new UTF8Encoding(true).GetBytes(tempString);
                outputFile.Write(test2, 0, test2.Length);
            }
        }
    }
}
