using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using uPLibrary.Networking.M2Mqtt;
using uPLibrary.Networking.M2Mqtt.Messages;
using System.Windows.Forms.DataVisualization.Charting;

namespace MQTTTest

{
    public partial class Form1 : Form
    {
        //MQTT
        MqttClient mqttClient;

        //plotting
        private List<Tuple<string, string>> variables = new List<Tuple<string, string>>();
        private List<float> plot_data = new List<float>();
        private string plot_var;
        const int maxData = 100;


        public Form1()
        {
            //Parse memory map file
            foreach (string line in System.IO.File.ReadLines(@"C:\Users\Sabrina\Documents\School\ECE4805\test.txt"))
            {
                string[] words = line.Split(' ');
                if (words.Length == 4)
                {
                    string var_name = words[3].Trim();
                    string var_loc = words[0].Trim();
                    if (var_name.StartsWith("gui"))
                    {
                        this.variables.Add(Tuple.Create(var_name, var_loc));
                    }
                }
            }

            //for testing purposes
            this.variables.Add(Tuple.Create("Value", "0"));
            this.variables.Add(Tuple.Create("Vab", "0"));
            this.variables.Add(Tuple.Create("Vbc", "0"));
            this.variables.Add(Tuple.Create("Vca", "0"));
            this.variables.Add(Tuple.Create("Vab_RMS", "0"));
            this.variables.Add(Tuple.Create("Vbc_RMS", "0"));
            this.variables.Add(Tuple.Create("Vca_RMS", "0"));

            InitializeComponent();

            //Put memory variables in drop-down menu
            comboBox1.DropDownStyle = ComboBoxStyle.DropDownList;
            comboBox1.MaxDropDownItems = variables.Count();
            comboBox1.Items.AddRange(variables.Select(x => x.Item1).ToList().ToArray());
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Task.Run(() =>
            {
                mqttClient = new MqttClient(textBox1.Text);                                 // create instance of the class
                mqttClient.MqttMsgPublishReceived += MqttClient_MqttMsgPublishReceived;     // assign callback function
                mqttClient.Connect("UniqueID", textBox2.Text, textBox3.Text);               // connect to a server
            });
        }

        private void MqttClient_MqttMsgPublishReceived(object sender, uPLibrary.Networking.M2Mqtt.Messages.MqttMsgPublishEventArgs e)
        {
            string message = Encoding.UTF8.GetString(e.Message);
            string topic = e.Topic;

            //Add message to plot
            if (topic.Equals(plot_var)) { 
                listBox1.Invoke((MethodInvoker)(() => listBox1.Items.Add(message)));
                plot_data.Add(float.Parse(message));
                if (plot_data.Count() > maxData)
                {
                    plot_data.RemoveAt(0);
                }
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            mqttClient.Disconnect();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            //connect and disconnect buttons
            if (mqttClient!=null)
            {
                button1.Enabled = !mqttClient.IsConnected;
                button2.Enabled = mqttClient.IsConnected;
            }
            if(chart1.Series[0] != null)
            {
                chart1.Series[0].Points.DataBindY(plot_data);
            }
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            mqttClient.Disconnect();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            plot_var = String.Format("{0}", this.comboBox1.SelectedItem);
            string title = String.Format("Plotting: {0}", plot_var);

            // subscribe to topic
            mqttClient.Subscribe(new string[] { plot_var }, new byte[] { MqttMsgBase.QOS_LEVEL_AT_LEAST_ONCE });

            // create a series for each line
            Series series1 = new Series(plot_var);
            series1.Points.DataBindY(plot_data);
            series1.ChartType = SeriesChartType.FastLine;

            // add each series to the chart
            chart1.Series.Clear();
            chart1.Series.Add(series1);

            // additional styling
            chart1.ResetAutoValues();
            chart1.Titles.Clear();
            chart1.Titles.Add(title);
            chart1.ChartAreas[0].AxisX.Title = "Horizontal Axis Label";
            chart1.ChartAreas[0].AxisY.Title = "Vertical Axis Label";
            chart1.ChartAreas[0].AxisY.MajorGrid.LineColor = Color.LightGray;
            chart1.ChartAreas[0].AxisX.MajorGrid.LineColor = Color.LightGray;
        }
    }
}
