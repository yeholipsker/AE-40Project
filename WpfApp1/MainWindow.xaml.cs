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
        IPEndPoint endPointStreamer, endPointReceiver;
        TcpClient clientStreamer, clientReceiver;
        NetworkStream stream, stream2;
        BinaryReader readerStreamer, readerReceiver;
        BinaryWriter writerStreamer, writerReceiver;
        
        public MainWindow()
        {
            InitializeComponent();
        }

        // Connect click
        private void Connect_Click(object sender, RoutedEventArgs e)
        {
            //connect to stream server
            endPointStreamer = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 4444); //new IPEndPoint(IPAddress.Parse(IP.Text.ToString()), Convert.ToInt32(Port.Text.ToString()));
            clientStreamer = new TcpClient();
            clientStreamer.Connect(endPointStreamer);
            stream = clientStreamer.GetStream();
            stream.ReadTimeout = 5000;
            readerStreamer = new BinaryReader(stream);
            writerStreamer = new BinaryWriter(stream);

            endPointReceiver = new IPEndPoint(IPAddress.Parse(IP.Text.ToString()), 8000);
            clientReceiver = new TcpClient();
            clientReceiver.Connect(endPointReceiver);
            stream2 = clientReceiver.GetStream();
            stream2.ReadTimeout = 5000;
            readerReceiver = new BinaryReader(stream2);
            writerReceiver = new BinaryWriter(stream2);
        }

        // Create a Json representation of the action.
        private JObject ActionToJson(String ip, int port, String action)
        {
            // Build the JSON action to return.
            JObject jsonAction = new JObject();
            jsonAction["IP"] = ip;
            jsonAction["Port"] = port;
            jsonAction["Action"] = action;

            Console.WriteLine(jsonAction);

            return jsonAction;
        }

        private String GetLocalIPAddress()
        {
            String ipAddr = "";
            var host = Dns.GetHostEntry(Dns.GetHostName());
            foreach (var ip in host.AddressList)
            {
                if (ip.AddressFamily == AddressFamily.InterNetwork)
                {
                    ipAddr = ip.ToString();
                }
            }
            return ipAddr;
        }

        // Action to send the server click.
        private void Action_Click(object sender, RoutedEventArgs e)
        {
            string content = (sender as Button).Content.ToString();

            // Build the JSON action to send.
            JObject jsonAction = ActionToJson(IP.Text.ToString(), Convert.ToInt32(Port.Text.ToString()), content);

            try
            {
                writerStreamer.Write(jsonAction.ToString().ToCharArray());
            } catch(IOException)
            {
                MessageBox.Show("No connection with Server");
            }

            if(content == "Check")
            {
                // Try to read the data and print a message about the connection
                byte[] buffer = new byte[clientStreamer.ReceiveBufferSize];
                int bytesRead;
                try
                {
                    bytesRead = stream.Read(buffer, 0, clientStreamer.ReceiveBufferSize);
                    MessageBox.Show("You have connection with server");
                }
                catch (IOException)
                {
                    MessageBox.Show("Connection timeout");
                }
            }
            if (content == "Start")
            {
                //throw new Exception("No network adapters with an IPv4 address in the system!");
                try
                {
                    JObject jsonActionToReceiver = ActionToJson(GetLocalIPAddress(), Convert.ToInt32(Port.Text.ToString()), content);
                    writerReceiver.Write(jsonActionToReceiver.ToString());
                }
                catch (IOException)
                {
                    MessageBox.Show("No connection with remote client");
                }
            }
        }
    }
}
