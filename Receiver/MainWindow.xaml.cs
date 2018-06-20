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


        // The main window of the receive part.
        public MainWindow()
        {
            InitializeComponent();
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
                vlcPlayer.MediaPlayer.Play(new FileInfo(SDP_PATH));
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
            IPEndPoint endPoint = new IPEndPoint(IPAddress.Parse("0.0.0.0"), 8000);
            TcpListener listener = new TcpListener(endPoint);
            listener.Start();
            Task listenTask = new Task(() =>
                {
                    for (numberOfWindows = 1; numberOfWindows < 4; numberOfWindows++)
                    {
                        TcpClient client = listener.AcceptTcpClient();

                        //Task listenTask = new Task(() =>
                        //{
                        using (NetworkStream stream = client.GetStream())
                        using (BinaryReader reader = new BinaryReader(stream))
                        using (BinaryWriter writer = new BinaryWriter(stream))
                        {
                            String ip = reader.ReadString();
                            // Create sdp file.
                            using (var tw = new StreamWriter("connectionDetails" + (numberOfWindows + 1) + ".sdp", false))
                            {
                                tw.WriteLine("v=0\no=- 49452 4 IN IP4 " + ip + "\ns=Test MP3 session\ni=Parameters for the session streamed by \"testMP3Streamer\"\nt=0 0\na=tool:testMP3Streamer\na=type:broadcast\nm=audio 6666 RTP/AVP 14\nc=IN IP4 127.0.0.1\nm=video 8888 RTP/AVP 96\nc=IN IP4 127.0.0.1\na=rtpmap:96 H264/90000\na=fmtp:96 packetization-mode=1");
                            }
                            Dispatcher.Invoke(new Action(() => { AddNewWindow("connectionDetails" + numberOfWindows + ".sdp"); }));
                        }

                    }
                });
            listenTask.Start();
            /*
            Task createWindowTask = new Task(() =>
            {
                int next = 1;
                while (true)
                {
                    if (next == numberOfWindows)
                    {
                        AddNewWindow("connectionDetails" + numberOfWindows + ".sdp");
                        next++;
                    }
                }
            });
            createWindowTask.Start();
            */
        }
    }
}