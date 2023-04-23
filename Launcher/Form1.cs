using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Launcher {
    public partial class Form1 : Form {
        
        public Form1() {
            InitializeComponent();
            InitWindow();
        }

        void InitWindow() {
            // music.dat struct
            // 0x00 - 0x03: MAGIC (0x4D4F4F4E)
            // 0x04 - 0x07: Number of files
            // [null-terminated bytes]: directory path
            // foreach NumberOfFiles
            // [13 bytes] ojn file: o2maxxxx.ojn
            // [32 bytes] ojn name
            // [4*3 bytes] difficulty lvl

            if (File.Exists(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "music.dat"))) {
                // create binaryreader from music.dat
                using (BinaryReader br = new BinaryReader(File.Open(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "music.dat"), FileMode.Open))) {
                    // read magic
                    int magic = br.ReadInt32();
                    // read number of files
                    int numFiles = br.ReadInt32();

                    byte[] bytes = new byte[256];
                    // read until null
                    int i = 0;
                    while (br.PeekChar() != 0) {
                        bytes[i] = br.ReadByte();
                        i++;
                    }

                    // clear listbox
                    listBox1.Items.Clear();
                    // read each file
                    for (int i = 0; i < numFiles; i++) {
                        // read ojn file
                        string ojnFile = Encoding.ASCII.GetString(br.ReadBytes(13)).TrimEnd('\0');
                        // read ojn name
                        string ojnName = Encoding.ASCII.GetString(br.ReadBytes(32)).TrimEnd('\0');
                        // read difficulty lvl
                        for (int j = 0; j < 3; j++) {
                            int diff = br.ReadInt32();

                            if (diff != -1) {
                                string diffName = ojnName + " - Lvl. " + diff;
                                // add to listbox
                                listBox1.Items.Add(diffName);
                            }
                        }
                    }
                }
                
            } else {
                FolderBrowserDialog dialog = new FolderBrowserDialog();
                dialog.Description = "Select the music folder installed.";
                if (dialog.ShowDialog() == DialogResult.OK) {
                    // iterate folder
                    string[] files = Directory.GetFiles(dialog.SelectedPath, "o2ma*.ojn", SearchOption.TopDirectoryOnly);
                    if (files.Length == 0) {
                        MessageBox.Show("No music files found in the selected folder.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                        Environment.Exit(0);
                    }
                    
                    foreach (string file in files) {
                        
                    }
                } else {
                    MessageBox.Show("You must select the music folder to continue.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    Environment.Exit(0);
                }
            }
        }

        private void button1_Click(object sender, EventArgs e) {

        }

        private void button3_Click(object sender, EventArgs e) {

        }

        private void button2_Click(object sender, EventArgs e) {

        }
    }
}
