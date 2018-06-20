using System;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Newtonsoft.Json.Linq;

namespace Vlc.DotNet.Wpf.Samples
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow
    {
        // Consts
        public const String SDP_PATH = @"..\..\..\H264Mp3.sdp";
        public const String PATH = @"\..\..\VLC\";
        public const int MAX_NUMBER_OF_WINDOWS = 4;

        // Data member.
        public delegate void createWindowCallback(String sdpPath);
        private int numberOfWindows = 0;
        TcpListener listener = null;


        // The main window of the receive part.
        public MainWindow()
        {
            InitializeComponent();
            IPEndPoint endPoint = new IPEndPoint(IPAddress.Parse("0.0.0.0"), 8000);
            listener = new TcpListener(endPoint);
            listener.Start();
        }

        // 'Stop' button method.
        private void StopButton_Click(object sender, RoutedEventArgs e)
        {
            if (StopButton.Content.ToString() == "Stop")
            {
                // Stop the video.
                vlcPlayer.MediaPlayer.Stop();
                StopButton.Content = "Play";
            }
            else
            {
                // Play
                vlcPlayer.MediaPlayer.Play(new FileInfo(SDP_PATH)); // TODO - CHANGE THE URI
                StopButton.Content = "Stop";
            }
        }

        // 'Add new' button method.
        private void AddNewWindow(String sdpPath)
        {
            if (numberOfWindows <= MAX_NUMBER_OF_WINDOWS)
            {
                // Change the screen respectively to the number of windows.
                switch (numberOfWindows)
                {
                    case 1:
                        // Add new VlcPlayer.
                        vlcPlayer.MediaPlayer.VlcLibDirectory = new DirectoryInfo(Directory.GetCurrentDirectory() + PATH);
                        vlcPlayer.MediaPlayer.EndInit();
                        vlcPlayer.MediaPlayer.Play(new FileInfo(sdpPath));
                        break;
                    case 2:
                        // Split the screen into right & left.
                        ColumnDefinition newColTop = new ColumnDefinition();
                        newColTop.Width = new GridLength(400);
                        ScreenGridTop.ColumnDefinitions.Add(newColTop);

                        // Add new VlcPlayer.
                        vlcPlayer2.MediaPlayer.VlcLibDirectory = new DirectoryInfo(Directory.GetCurrentDirectory() + PATH);
                        vlcPlayer2.MediaPlayer.EndInit();
                        vlcPlayer2.MediaPlayer.Play(new FileInfo(sdpPath));
                        break;
                    case 3:
                        // Split the screen into top & bottom.
                        RowDefinition newRow = new RowDefinition();
                        newRow.Height = new GridLength(250);
                        ScreenGrid.RowDefinitions.Add(newRow);

                        // Add new VlcPlayer.
                        vlcPlayer3.MediaPlayer.VlcLibDirectory = new DirectoryInfo(Directory.GetCurrentDirectory() + PATH);

                        vlcPlayer3.MediaPlayer.EndInit();
                        vlcPlayer3.MediaPlayer.Play(new FileInfo(sdpPath));
                        break;
                    case 4:
                        // Split the bottom half of the screen.
                        ColumnDefinition newColDown = new ColumnDefinition();
                        newColDown.Width = new GridLength(400);
                        ScreenGridDown.ColumnDefinitions.Add(newColDown);

                        // Add new VlcPlayer.
                        vlcPlayer4.MediaPlayer.VlcLibDirectory = new DirectoryInfo(Directory.GetCurrentDirectory() + PATH);

                        vlcPlayer4.MediaPlayer.EndInit();
                        vlcPlayer4.MediaPlayer.Play(new FileInfo(sdpPath));
                        break;
                    default:
                        break;
                }
            }
        }

        private void ListenButton_Click(object sender, RoutedEventArgs e)
        {
            TcpClient client = listener.AcceptTcpClient();
            using (NetworkStream stream = client.GetStream())
            using (BinaryReader reader = new BinaryReader(stream))
            using (BinaryWriter writer = new BinaryWriter(stream))
            {
                String details = reader.ReadString();
                JObject streamDetails = JObject.Parse(details);
                numberOfWindows++;
                // Create sdp file.
                using (var tw = new StreamWriter("connectionDetails" + numberOfWindows + ".sdp", false))
                {
                    tw.Write("v=0\n");
                    tw.Write("o=- 49452 4 IN IP4 " + streamDetails["IP"] + "\n");
                    tw.Write("s=Test MP3 session\n");
                    tw.Write("i=Parameters for the session streamed by \"testMP3Streamer\"\n");
                    tw.Write("t=0 0\n");
                    tw.Write("a=tool:testMP3Streamer\n");
                    tw.Write("a=type:broadcast\n");
                    tw.Write("m=audio " + streamDetails["Port"] + " RTP/AVP 14\n");
                    tw.Write("c=IN IP4 127.0.0.1\n");
                    tw.Write("m=video " + (Convert.ToInt32(streamDetails["Port"]) + 2) + " RTP/AVP 96\n");
                    tw.Write("c=IN IP4 127.0.0.1\n");
                    tw.Write("a=rtpmap:96 H264/90000\n");
                    tw.Write("a=fmtp:96 packetization-mode=1");
                }
                AddNewWindow("connectionDetails" + numberOfWindows + ".sdp");
            }
        }
    }
}