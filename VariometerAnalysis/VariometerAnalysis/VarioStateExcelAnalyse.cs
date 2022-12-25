using NPOI.HSSF.Record;
using NPOI.SS.UserModel;
using NPOI.SS.UserModel.Charts;
using NPOI.SS.Util;
using NPOI.XSSF.UserModel;
using ICell = NPOI.SS.UserModel.ICell;

namespace VariometerAnalysis
{
    public struct vario_state
    {
        public float height;
        public float accX;
        public float velX;
        public float[] rotation;
        public Int64 time;
    }
    class VarioStateExcelAnalyse
    {
        public static void AnalyzeFlug(String filePath)
        {
            IWorkbook wb = new XSSFWorkbook();
            ISheet sheet = wb.CreateSheet("linechart");

            IRow row;
            const int MAX_BUFFER = 1048576; //1MB 
            byte[] buffer = new byte[MAX_BUFFER];
            int bytesRead;
            List<vario_state> vario_states_read = new List<vario_state>();
            int count = 0;
            using (FileStream fs = File.Open(filePath, FileMode.Open, FileAccess.Read))
            {
                while ((bytesRead = fs.Read(buffer, 0, MAX_BUFFER)) != 0) //reading 1mb chunks at a time
                {
                    int readPos = 37 * 00;
                    while (readPos < MAX_BUFFER)
                    {
                        bool foundNotZero = false;
                        for (int i = 0; i < 60; i++)
                        {
                            if (i + readPos < MAX_BUFFER && buffer[i + readPos] != 0)
                                foundNotZero = true;
                        }
                        if (!foundNotZero)
                            break;
                        row = sheet.CreateRow((short)count);
                        count++;
                        vario_state tempState = new vario_state();
                        tempState.height = BitConverter.ToSingle(buffer, readPos); // Convert the bytes to a float
                        setCell(row, 0, tempState.height);
                        readPos += 4;
                        tempState.velX = BitConverter.ToSingle(buffer, readPos);
                        setCell(row, 1, tempState.velX);
                        readPos += 4;
                        tempState.accX = BitConverter.ToSingle(buffer, readPos);
                        setCell(row, 2, tempState.accX);
                        readPos += 4;
                        tempState.rotation = new float[4];
                        tempState.rotation[0] = BitConverter.ToSingle(buffer, readPos);
                        readPos += 4;
                        tempState.rotation[1] = BitConverter.ToSingle(buffer, readPos);
                        readPos += 4;
                        tempState.rotation[2] = BitConverter.ToSingle(buffer, readPos);
                        readPos += 4;
                        tempState.rotation[3] = BitConverter.ToSingle(buffer, readPos);
                        readPos += 4;
                        tempState.time = BitConverter.ToInt64(buffer, readPos) / 1000;
                        setCell(row, 3, tempState.time);
                        readPos += 8;
                        char test = BitConverter.ToChar(buffer, readPos);
                        readPos += 1;
                        vario_states_read.Add(tempState);
                    }
                }
            }

            IDrawing drawing = sheet.CreateDrawingPatriarch();
            IClientAnchor anchor1 = drawing.CreateAnchor(0, 0, 0, 0, 0, 10, 10, 40);
            CreateChart(drawing, sheet, anchor1, "title1", "title2", count);
            //IClientAnchor anchor2 = drawing.CreateAnchor(0, 0, 0, 0, 0, 20, 10, 35);
            //CreateChart(drawing, sheet, anchor2, "s1", "s2", count, true);
            using (FileStream fs = File.Create("../../../test.xlsx"))
            {
                wb.Write(fs);
            }
        }
        static void CreateChart(IDrawing drawing, ISheet sheet, IClientAnchor anchor, string serie1, string serie2, int NUM_OF_COLUMNS, bool enableMajorGridline = false)
        {
            XSSFChart chart = (XSSFChart)drawing.CreateChart(anchor);
            //chart.SetTitle("Test 1");
            IChartLegend legend = chart.GetOrCreateLegend();
            legend.Position = LegendPosition.Bottom;

            ILineChartData<double, double> data = chart.ChartDataFactory.CreateLineChartData<double, double>();

            // Use a category axis for the bottom axis.
            IChartAxis bottomAxis = chart.ChartAxisFactory.CreateCategoryAxis(AxisPosition.Bottom);
            IValueAxis leftAxis = chart.ChartAxisFactory.CreateValueAxis(AxisPosition.Left);
            leftAxis.Maximum = 0.5f;
            leftAxis.Minimum = -0.5f;
            leftAxis.Crosses = AxisCrosses.AutoZero;

            IChartDataSource<double> xs = DataSources.FromNumericCellRange(sheet, new CellRangeAddress(3, 3, 0, NUM_OF_COLUMNS - 1));
            IChartDataSource<double> ys1 = DataSources.FromNumericCellRange(sheet, new CellRangeAddress(0, 0, 0, NUM_OF_COLUMNS - 1));
            //IChartDataSource<double> ys2 = DataSources.FromNumericCellRange(sheet, new CellRangeAddress(1, 1, 0, NUM_OF_COLUMNS - 1));

            var s1 = data.AddSeries(xs, ys1);
            s1.SetTitle(serie1);
            //var s2 = data.AddSeries(xs, ys2);
            //s2.SetTitle(serie2);
            chart.Plot(data, bottomAxis, leftAxis);
            //add major gridline, available since NPOI 2.5.5
            //var plotArea = chart.GetCTChart().plotArea;
            //plotArea.catAx[0].AddNewMajorGridlines();
            //plotArea.valAx[0].AddNewMajorGridlines();

        }

        static void setCell(IRow row, int cellIndex, float value)
        {
            ICell cell = row.CreateCell(cellIndex);
            cell.SetCellValue(value);
        }
    }
}
