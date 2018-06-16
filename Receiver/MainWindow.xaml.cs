using System;
using System.IO;
using System.Windows;
using System.Windows.Controls;

namespace Vlc.DotNet.Wpf.Samples
{
    public partial class MainWindow
    {
        public const String RTP_ADDRESS = @"rtp:\\@224.1.1.1:5004";
        public const String FILE_ADDRESS = @"C:\Users\User\Downloads\100_3392.JPG";
        public const String PATH = @"\..\..\VLC\";
        public const int MAX_NUMBER_OF_WINDOWS = 4;

        private int numberOfWindows = 0;

        public MainWindow()
        {
            InitializeComponent();
            vlcPlayer.MediaPlayer.VlcLibDirectory =
                new DirectoryInfo(Directory.GetCurrentDirectory() + PATH);

            vlcPlayer.MediaPlayer.EndInit();
            vlcPlayer.MediaPlayer.Play(new Uri(@"C:\Users\User\Desktop\1.png"));
            numberOfWindows++;
        }

        private void StopButton_Click(object sender, RoutedEventArgs e)
        {
            
            if (StopButton.Content.ToString() == "Stop")
            {
                vlcPlayer.MediaPlayer.Stop();
                StopButton.Content = "Play";
            }
            else
            {
                vlcPlayer.MediaPlayer.Play(new Uri(RTP_ADDRESS));
                StopButton.Content = "Stop";
            }
        }

        private void AddNewButton_Click(object sender, RoutedEventArgs e)
        {
            if (++numberOfWindows <= MAX_NUMBER_OF_WINDOWS)
            {
                // Add new VlcPlayer.
                VlcControl player = new VlcControl();
                player.MediaPlayer.VlcLibDirectory =
                    new DirectoryInfo(Directory.GetCurrentDirectory() + PATH);

                player.BeginInit();
                player.EndInit();

                // Change the screen.
                Console.WriteLine("numberOfWindows = " + numberOfWindows);
                switch (numberOfWindows)
                {
                    case 2:
                        ColumnDefinition newColTop = new ColumnDefinition();
                        newColTop.Width = new GridLength(400);
                        ScreenGridTop.ColumnDefinitions.Add(newColTop);

                        vlcPlayer2.MediaPlayer.VlcLibDirectory =
                            new DirectoryInfo(Directory.GetCurrentDirectory() + PATH);
                        vlcPlayer2.MediaPlayer.EndInit();
                        vlcPlayer2.MediaPlayer.Play(new Uri(@"C:\Users\User\Desktop\2.png"));

                        break;

                    case 3:
                        RowDefinition newRow = new RowDefinition();
                        newRow.Height = new GridLength(250);
                        ScreenGrid.RowDefinitions.Add(newRow);

                        vlcPlayer3.MediaPlayer.VlcLibDirectory =
                            new DirectoryInfo(Directory.GetCurrentDirectory() + PATH);

                        vlcPlayer3.MediaPlayer.EndInit();
                        vlcPlayer3.MediaPlayer.Play(new Uri(@"C:\Users\User\Desktop\3.png"));
                        break;
                    case 4:
                        ColumnDefinition newColDown = new ColumnDefinition();
                        newColDown.Width = new GridLength(400);
                        ScreenGridDown.ColumnDefinitions.Add(newColDown);

                        vlcPlayer4.MediaPlayer.VlcLibDirectory =
                            new DirectoryInfo(Directory.GetCurrentDirectory() + PATH);

                        vlcPlayer4.MediaPlayer.EndInit();
                        vlcPlayer4.MediaPlayer.Play(new Uri(@"C:\Users\User\Desktop\4.png"));
                        break;
                    default:
                        break;
                }
            }
        }
    }
}