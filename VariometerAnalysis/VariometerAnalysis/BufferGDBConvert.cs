using NPOI.Util;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VariometerAnalysis
{
    class BufferGDBConvert
    {
        public static void ConvertGDBBuffer(String filePath)
        {
            using (FileStream fs = File.Open(filePath, FileMode.Open, FileAccess.Read))
            {
                using (StreamReader reader = new StreamReader(fs))
                {
                    reader.Read();
                    string currByte = "";
                    List<int> numbersInArray = new List<int>();
                    while (!reader.EndOfStream)
                    {
                        char nextChar = (char)reader.Read();
                        if(nextChar == '}')
                        {
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
                        }
                        //read repeat Byte
                        if(nextChar == ' ')
                        {
                            int HexByte = Convert.ToInt32(currByte, 16);
                            numbersInArray.Add(HexByte);
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
                            int HexByte = Convert.ToInt32(currByte, 16);
                            numbersInArray.Add(HexByte);
                            currByte = "";
                            char test1 = (char)reader.Read();
                        }
                        else
                        {
                            currByte += nextChar;
                        }
                    }
                }
            }
        }
    }
}
