using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Windows;
using System.Windows.Controls;
using Newtonsoft.Json.Linq;

namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        // Data members.
        IPEndPoint endPoint;
        TcpClient client;
        NetworkStream stream;
        BinaryReader reader;
        BinaryWriter writer;

        // The main window of the GUI.
        public MainWindow()
        {
            InitializeComponent();
        }

        // Connect click.
        private void Connect_Click(object sender, RoutedEventArgs e)
        {
            // Conect to the server.
            endPoint = new IPEndPoint(IPAddress.Parse(IP.Text.ToString()), Convert.ToInt32(Port.Text.ToString()));
            client = new TcpClient();
            client.Connect(endPoint);

            stream = client.GetStream();
            stream.ReadTimeout = 5000;
            reader = new BinaryReader(stream);
            writer = new BinaryWriter(stream);
        }

        // Create a JSON representation of the action.
        private JObject ActionToJson(String action)
        {
            // Build the JSON action to return.
            JObject jsonAction = new JObject();
            jsonAction["IP"] = IP.Text.ToString();
            jsonAction["Port"] = Convert.ToInt32(Port.Text.ToString());
            jsonAction["Action"] = action;

            Console.WriteLine(jsonAction);

            return jsonAction;
        }

        // Action to send the server click.
        private void Action_Click(object sender, RoutedEventArgs e)
        {
            string content = (sender as Button).Content.ToString();

            // Build the JSON action to send.
            JObject jsonAction = ActionToJson(content);

            try
            {
                writer.Write(jsonAction.ToString().ToCharArray());
            } catch(IOException)
            {
                MessageBox.Show("No connection with Server");
            }

            if(content == "Check")
            {
                // Try to read the data and print a message about the connection
                byte[] buffer = new byte[client.ReceiveBufferSize];
                int bytesRead;
                try
                {
                    bytesRead = stream.Read(buffer, 0, client.ReceiveBufferSize);
                    MessageBox.Show("You have connection with server");
                }
                catch (IOException)
                {
                    MessageBox.Show("Connection timeout");
                }
            }
        }
    }
}
