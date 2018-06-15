using System;
using System.IO;
using System.Windows;
using System.Windows.Controls;

namespace Vlc.DotNet.Wpf.Samples
{
    public partial class MainWindow
    {
        public const String RTP_ADDRESS = @"rtp:\\@224.1.1.1:5004";
        public const int maxNumberOfWindows = 4;

        private int numberOfWindows = 0;

        public MainWindow()
        {
            InitializeComponent();
            vlcPlayer.MediaPlayer.VlcLibDirectory =
                new DirectoryInfo(Directory.GetCurrentDirectory() + @"\..\..\VLC\");

            vlcPlayer.MediaPlayer.EndInit();
            vlcPlayer.MediaPlayer.Play(new Uri(RTP_ADDRESS));
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

            // ==================== THE ADD PLAYER PART ==================== //
            if (++numberOfWindows >= maxNumberOfWindows)
            {

            }

            // Add new VlcPlayer.
            VlcControl player = new VlcControl();
            player.MediaPlayer.VlcLibDirectory =
                new DirectoryInfo(Directory.GetCurrentDirectory() + @"\..\..\VLC\");

            player.BeginInit();
            player.EndInit();
            vlcPlayer.MediaPlayer.Play(new Uri(RTP_ADDRESS));

            // Change the screen.
            switch (numberOfWindows)
            {
                case 2:
                    Console.WriteLine("Case 2");
                    ColumnDefinition newCol = new ColumnDefinition();
                    newCol.Width = new GridLength(400);
                    ScreenGrid.ColumnDefinitions.Add(newCol);
                    ScreenGrid.Children.Add(player);
                    Grid.SetColumn(player, ScreenGrid.RowDefinitions.Count);
                    break;
                case 3:
                    Console.WriteLine("Case 3");
                    RowDefinition newRow = new RowDefinition();
                    newRow.Height = new GridLength(250);
                    ScreenGrid.RowDefinitions.Add(newRow);
                    ScreenGrid.Children.Add(player);
                    Grid.SetRow(player, ScreenGrid.RowDefinitions.Count);
                    Grid.SetColumn(player, ScreenGrid.RowDefinitions.Count);
                    break;
                case 4:
                    Console.WriteLine("Case 4");
                    ScreenGrid.Children.Add(player);
                    Grid.SetRow(player, ScreenGrid.RowDefinitions.Count);
                    Grid.SetColumn(player, ScreenGrid.RowDefinitions.Count - 2);
                    break;
                default:
                    break;
            }

            
            player.MediaPlayer.Play(new Uri(RTP_ADDRESS));

            // ============================================================== //
            

        }
    }
}