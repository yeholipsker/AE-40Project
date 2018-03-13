using System;
using System.Collections.Generic;
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
using Newtonsoft.Json.Linq;

namespace WpfApp1
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }
        
        private JObject getAction()
        {
            // Build the JSON action to return.
            JObject jsonAction = new JObject();
            jsonAction["IP"] = IP.Text.ToString();
            jsonAction["Port"] = Port.Text.ToString();

            return jsonAction;
        }

        private void Start_Click(object sender, RoutedEventArgs e)
        {
            // Build the JSON action to send.
            JObject jsonAction = getAction();
            jsonAction["Action"] = Start.Content.ToString();

            Console.WriteLine(jsonAction);
        }

        private void Stop_Click(object sender, RoutedEventArgs e)
        {
            // Build the JSON action to send.
            JObject jsonAction = getAction();
            jsonAction["Action"] = Stop.Content.ToString();

            Console.WriteLine(jsonAction);
        }

        private void Check_Click(object sender, RoutedEventArgs e)
        {
            // Build the JSON action to send.
            JObject jsonAction = getAction();
            jsonAction["Action"] = Check.Content.ToString();

            Console.WriteLine(jsonAction);
        }
    }
}
