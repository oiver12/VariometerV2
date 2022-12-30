using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WindowsFormsApp1
{
    public enum EInkColor
    {
        Black,
        White
    }
    public partial class Form1 : Form
    {
        static Graphics g;
        static PictureBox BitmapPictureBox;
        static Bitmap bitmap;
        public Form1()
        {
            InitializeComponent();
            BitmapPictureBox = new PictureBox();
            Controls.Add(BitmapPictureBox);
            ImageToBitMapBuffer.saveBufferImage("../../Kresi_Vario.png");
            BufferGDBConvert.ConvertGDBBuffer("../../OutputFileBuffer.txt");
            System.Diagnostics.Process.Start("notepad.exe", "../../OutputFileBuffer.txt");
        }

        public static void StartDraw(int sizeX, int sizeY)
        {
            BitmapPictureBox.Size = new Size(sizeX + 1, sizeY + 1);
            bitmap = new Bitmap(sizeX + 1, sizeY + 1);
            BitmapPictureBox.Image = bitmap;
            for (int y = 0; y < sizeY + 1; y++)
            {
                bitmap.SetPixel(sizeX, y, Color.Black);
            }
            for (int x = 0; x < sizeX + 1; x++)
            {
                bitmap.SetPixel(x, sizeY, Color.Black);
            }
        }

        public static void SetPixelBitMap(int x, int y, EInkColor color)
        {
            bitmap.SetPixel(x, y, color == EInkColor.Black ? Color.Black : Color.White);
        }

        public static void SetPixel(int x, int y, Color color)
        {
            bitmap.SetPixel(x, y, color);
        }
    }
}
