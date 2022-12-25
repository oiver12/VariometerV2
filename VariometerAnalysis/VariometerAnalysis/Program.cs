using System;
using System.IO;
using System.Text;


namespace VariometerAnalysis
{
    internal class Program
    {
        static void Main(string[] args)
        {
            BufferGDBConvert.ConvertGDBBuffer("../../../ArrayAnalyse.txt");
        }
    }
}
