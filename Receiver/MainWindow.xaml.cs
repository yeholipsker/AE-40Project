using System;
using System.IO;
using System.Reflection;
using System.Windows;

namespace Vlc.DotNet.Wpf.Samples
{
    public partial class MainWindow
    {
        public const String RTP = "rtp:\\\\@224.1.1.1:5004";

        public MainWindow()
        {
            InitializeComponent();
            vlcPlayer.MediaPlayer.VlcLibDirectory =
                new DirectoryInfo(@"c:\Program Files (x86)\VideoLAN\VLC\"); //TODO - NEED TO CHANGE THIS SOMEHOW...

            vlcPlayer.MediaPlayer.EndInit();
            vlcPlayer.MediaPlayer.Play(new Uri(RTP));
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
                vlcPlayer.MediaPlayer.Play(new Uri(RTP));
                StopButton.Content = "Stop";
            }
        }
    }
}








/*
using System;
using System.IO;
using System.Reflection;
using System.Windows;

namespace Vlc.DotNet.Wpf.Samples
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            myControl.MediaPlayer.VlcLibDirectoryNeeded += OnVlcControlNeedsLibDirectory;
            myControl.MediaPlayer.EndInit();

            // This can also be called before EndInit
            this.myControl.MediaPlayer.Log += (sender, args) =>
            {
                System.Diagnostics.Debug.WriteLine(string.Format("libVlc : {0} {1} @ {2}", args.Level, args.Message, args.Module));
            };
        }

        private void OnVlcControlNeedsLibDirectory(object sender, Forms.VlcLibDirectoryNeededEventArgs e)
        {
            var currentAssembly = Assembly.GetEntryAssembly();
            var currentDirectory = new FileInfo(currentAssembly.Location).DirectoryName;
            if (currentDirectory == null)
                return;
            if (IntPtr.Size == 4)
                e.VlcLibDirectory = new DirectoryInfo(Path.Combine(currentDirectory, @"..\..\..\lib\x86\"));
            else
                e.VlcLibDirectory = new DirectoryInfo(Path.Combine(currentDirectory, @"..\..\..\lib\x64\"));
            Console.WriteLine(e.VlcLibDirectory);
        }

        private void OnPlayButtonClick(object sender, RoutedEventArgs e)
        {
            myControl.MediaPlayer.Play(new Uri("http://download.blender.org/peach/bigbuckbunny_movies/big_buck_bunny_480p_surround-fix.avi"));
            //myControl.MediaPlayer.Play(new FileInfo(@"..\..\..\Vlc.DotNet\Samples\Videos\BBB trailer.mov"));
        }

        private void OnForwardButtonClick(object sender, RoutedEventArgs e)
        {
            myControl.MediaPlayer.Rate = 2;
        }

        private void GetLength_Click(object sender, RoutedEventArgs e)
        {
            GetLength.Content = myControl.MediaPlayer.Length + " ms";
        }

        private void GetCurrentTime_Click(object sender, RoutedEventArgs e)
        {
            GetCurrentTime.Content = myControl.MediaPlayer.Time + " ms";
        }

        private void SetCurrentTime_Click(object sender, RoutedEventArgs e)
        {
            myControl.MediaPlayer.Time = 5000;
            SetCurrentTime.Content = myControl.MediaPlayer.Time + " ms";
        }
    }
}















/*

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
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
using System.Reflection;
using System.Windows;

using Vlc.DotNet.Wpf;

namespace Receiver
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private string currentDirectory = "\\";

        public MainWindow()
        {
            InitializeComponent();
            var vlcLibDirectory = new DirectoryInfo(System.IO.Path.Combine(currentDirectory, "libvlc", IntPtr.Size == 4 ? "win-x86" : "win-x64"));

            var options = new string[]
            {
                // VLC options can be given here. Please refer to the VLC command line documentation.
                 "-vvv --extraintf=logger --verbose=2 --logfile=Logs.log"
            };

            //this.MyControl.SourceProvider.CreatePlayer(vlcLibDirectory, options);

            // Load libvlc libraries and initializes stuff. It is important that the options (if you want to pass any) and lib directory are given before calling this method.
            this.MyControl.MediaPlayer.Play("http://download.blender.org/peach/bigbuckbunny_movies/big_buck_bunny_480p_h264.mov");

        }
        /*
        private void OnVlcControlNeedsLibDirectory(object sender, Vlc.DotNet.Forms.VlcLibDirectoryNeededEventArgs e)
        {
            var currentAssembly = Assembly.GetEntryAssembly();
            var currentDirectory = new FileInfo(currentAssembly.Location).DirectoryName;
            if (currentDirectory == null)
                return;
            if (IntPtr.Size == 4)
                e.VlcLibDirectory = new DirectoryInfo(System.IO.Path.Combine(currentDirectory, @"..\..\..\lib\x86\"));
            else
                e.VlcLibDirectory = new DirectoryInfo(System.IO.Path.Combine(currentDirectory, @"..\..\..\lib\x64\"));

            Console.WriteLine(System.IO.Path.Combine(currentDirectory, @"..\..\..\lib\x64\"));
        }*/
/*   }
   }
}
*/
