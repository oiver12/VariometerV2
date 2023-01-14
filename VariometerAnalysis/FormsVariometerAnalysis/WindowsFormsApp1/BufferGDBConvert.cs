//#define WriteToNewFile 
#define ShowOnScreen
//#define Buffer_lvcolor

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

/*
 * Commands :
 * -exec set max-value-size unlimited
 * -exec set print elements 0
 * -exec p/x (uint8_t[134400])*buffer_1Byte
 */

namespace WindowsFormsApp1
{ 
    class BufferGDBConvert
    {
        public static void ConvertGDBBuffer(String filePath, int sizeXVar, int sizeYVar)
        {
            using (FileStream fs = File.Open(filePath, FileMode.Open, FileAccess.Read))
            {
                using (StreamReader reader = new StreamReader(fs))
                {
                    reader.Read();
                    string currByte = "";
                    List<Byte> numbersInArray = new List<Byte>();
                    while ((char)reader.Read() != '{') { } ;
                    while (!reader.EndOfStream)
                    {
                        char nextChar = (char)reader.Read();
                        //Buffer Read finished
                        if(nextChar == '}')
                        {
                            Byte HexByte = Convert.ToByte(currByte, 16);
                            numbersInArray.Add(HexByte);
#if WriteToNewFile
                            using (FileStream outputFile = File.Open("../../../OutputFile.txt", FileMode.Open, FileAccess.Write))
                            {
                                string tempString = "{";
                                byte[] tempInfo = new UTF8Encoding(true).GetBytes(tempString);
                                outputFile.Write(tempInfo, 0, tempInfo.Length);
                                for (int i = 0; i < numbersInArray.Count; i++)
                                {
                                    tempString = $"0x{numbersInArray[i]:X}";
                                    if (i < numbersInArray.Count -1)
                                        tempString += ", ";
                                    byte[] info = new UTF8Encoding(true).GetBytes(tempString);
                                    outputFile.Write(info, 0, info.Length);
                                }
                                tempString = "}";
                                byte[] test2 = new UTF8Encoding(true).GetBytes(tempString);
                                outputFile.Write(test2, 0, test2.Length);
                            }
#endif
#if ShowOnScreen
                            int x1 = 0;
                            int x2 = sizeXVar;
                            int y1 = 0;
                            int y2 = sizeYVar;
                            int sizeX = (x2-x1) % 8 == 0 ? (x2 - x1 + 1) / 8 : (x2 - x1 + 1) / 8 + 1;
                            int sizeY = y2 - y1;
#if Buffer_lvcolor
                            int x1 = 0;
                            int x2 = 479;
                            int y1 = 40;
                            int y2 = 79;

                            /*y1 = (y1 / 8) * 8;
                            y2 = ((y2 + 8 - 1) & -8) - 1;*/
#endif
                            Form1.StartDraw(480, 280);
#if Buffer_lvcolor
                            for (int y = y1; y < y2; y++)
                            {
                                for (int x = x1; x < x2; x++)
                                {
                                    int addr = (x - x1) + ((y - y1) * (x2-x1+1));
                                    if (numbersInArray[addr] == 0)
                                        Form1.SetPixelBitMap(x, y, EInkColor.Black);
                                    else
                                        Form1.SetPixelBitMap(x, y, EInkColor.White);
#else
                            for (int y = y1; y < y2; y++)
                            {
                                for (int x = x1; x < x2; x++)
                                {
                                    int addr = (x - x1) / 8 + (y-y1) * sizeX;
                                    Byte rdata = numbersInArray[addr];
                                    //Black
                                    if((rdata & (0x80 >> ((x-x1) % 8))) == 0)
                                    {
                                        Form1.SetPixelBitMap(x, y, WindowsFormsApp1.EInkColor.Black);
                                    }
                                    else
                                    {
                                        Form1.SetPixelBitMap(x, y, WindowsFormsApp1.EInkColor.White);
                                    }
#endif
                                }
                            }
#endif
                        }
                        //read repeat Byte
                        if (nextChar == ' ')
                        {
                            Byte HexByte = Convert.ToByte(currByte, 16);                                 
                            while ((char)reader.Read() != ' ') ;
                            string repeatNumber = "";
                            while(reader.Peek() != ' ')                                      
                            {
                                repeatNumber += (char)reader.Read();
                            }
                            int numberToRepeat = 0;
                            Int32.TryParse(repeatNumber, out numberToRepeat);
                            for (int i = 0; i < numberToRepeat; i++)
                            {
                                numbersInArray.Add(HexByte);
                            }
                            reader.Read();
                            while ((char)reader.Read() != '>') ;
                            if(reader.Peek() != '}')
                            {
                                reader.Read();
                                reader.Read();
                            }
                            currByte = "";
                        }
                        //read nextByte
                        else if(nextChar == ',')
                        {
                            Byte HexByte = Convert.ToByte(currByte, 16);
                            numbersInArray.Add(HexByte);
                            currByte = "";
                            char test1 = (char)reader.Read();
                        }
                        else if(nextChar != '\n' && nextChar != '\r')
                        {
                            currByte += nextChar;
                        }
                    }
                }
            }
        }
    }
}
