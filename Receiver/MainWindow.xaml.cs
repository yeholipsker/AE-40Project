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
        private object currentDirectory;

        public MainWindow()
        {
            InitializeComponent();
            MyControl.MediaPlayer.VlcLibDirectoryNeeded += OnVlcControlNeedsLibDirectory;
            MyControl.MediaPlayer.EndInit();

            // This can also be called before EndInit
            this.MyControl.MediaPlayer.Log += (sender, args) =>
            {
                System.Diagnostics.Debug.WriteLine(string.Format("libVlc : {0} {1} @ {2}", args.Level, args.Message, args.Module));
            };

        }

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
        }
    }
}
