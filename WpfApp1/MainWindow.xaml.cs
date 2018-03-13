using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Newtonsoft.Json.Linq;

namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        IPEndPoint endPoint;
        TcpClient client;
        NetworkStream stream;
        BinaryReader reader;
        BinaryWriter writer;
        
        public MainWindow()
        {
            InitializeComponent();
        }

        private void Connect_Click(object sender, RoutedEventArgs e)
        {
            endPoint = new IPEndPoint(IPAddress.Parse(IP.Text.ToString()), Convert.ToInt32(Port.Text.ToString()));
            client = new TcpClient();
            client.Connect(endPoint);

            stream = client.GetStream();
            stream.Flush(); //TODO - REMOVE
            reader = new BinaryReader(stream);
            writer = new BinaryWriter(stream);
        }
       
        private JObject getAction(String action)
        {
            // Build the JSON action to return.
            JObject jsonAction = new JObject();
            jsonAction["IP"] = IP.Text.ToString();
            jsonAction["Port"] = Convert.ToInt32(Port.Text.ToString());
            jsonAction["Action"] = action;

            Console.WriteLine(jsonAction);

            return jsonAction;
        }

        private void Start_Click(object sender, RoutedEventArgs e)
        {
            // Build the JSON action to send.
            JObject jsonAction = getAction(Start.Content.ToString());

            writer.Write(jsonAction.ToString().ToCharArray());
        }

        private void Stop_Click(object sender, RoutedEventArgs e)
        {
            // Build the JSON action to send.
            JObject jsonAction = getAction(Stop.Content.ToString());

            writer.Write(jsonAction.ToString().ToCharArray());
        }

        private void Check_Click(object sender, RoutedEventArgs e)
        {
            // Build the JSON action to send.
            JObject jsonAction = getAction(Check.Content.ToString());

            writer.Write(jsonAction.ToString().ToCharArray());

            //read the data
            byte[] buffer = new byte[client.ReceiveBufferSize];
            int bytesRead = stream.Read(buffer, 0, client.ReceiveBufferSize);
            string dataReceived = Encoding.ASCII.GetString(buffer, 0, bytesRead);
            Console.WriteLine(dataReceived);
        }
    }
}
