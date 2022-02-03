﻿using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.ComponentModel;
using System.Threading;

namespace FaceMask
{
    public partial class Form1 : Form
    {

        [DllImport("FFMPegModule.dll", EntryPoint = "ReadVideo", CallingConvention = CallingConvention.Cdecl)]
        private static extern int ReadVideo(string strInputFile);

        [DllImport("FFMPegModule.dll", EntryPoint = "WriteVideo", CallingConvention = CallingConvention.Cdecl)]
        private static extern int WriteVideo(string strOutputFile);

        [DllImport("FFMPegModule.dll", EntryPoint = "IsRun", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool IsRun();

        [DllImport("FFMPegModule.dll", EntryPoint = "Process", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Process(string strInputFile, string strOutputFile);

        [DllImport("FFMPegModule.dll", EntryPoint = "Release", CallingConvention = CallingConvention.Cdecl)]
        private static extern void Release();

        //private int status;
        //[DllImport("FFMPegModule.dll", EntryPoint = "Test", CallingConvention = CallingConvention.Cdecl)]
        //private static extern int Test();

        //private string _inputFileName;

        //private string _outputFileName;

        public Form1()
        {
            InitializeComponent();
        }

        private void btnSelect_Click(object sender, EventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            if (openFileDialog.ShowDialog() == DialogResult.OK)
                doProcess(openFileDialog.FileName);
        }

        private void btnSelect_DragDrop(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
            doProcess(files[0]);
        }

        private void btnSelect_DragEnter(object sender, DragEventArgs e)
        {
            string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
            string ext = Path.GetExtension(files[0]);

            if (ext == ".mp4" || ext == ".h264")
                e.Effect = DragDropEffects.Copy;
            else
                e.Effect = DragDropEffects.None;
        }

        private void doProcess(string srcPath)
        {
            MessageBox.Show("파일 저장 대상을 선택하십시오.");
            SaveFileDialog dlg = new SaveFileDialog();
            string ext = Path.GetExtension(srcPath);
            dlg.FileName = Path.GetFileNameWithoutExtension(srcPath) + "_output";
            //dlg.DefaultExt = ext;
            //dlg.Filter = "MP4 비디오|*.mp4|AVI 비디오|*.avi";
            dlg.Filter = "H264 비디오|*.h264";//|MP4 비디오|*.mp4|AVI 비디오|*.avi";
            if (dlg.ShowDialog() == DialogResult.OK)
            {
                string dstPath = dlg.FileName;
                int ret = Process(srcPath, dstPath);
                if (ret < 0)
                    MessageBox.Show("Error on reading input video!");
                else if (ret > 0)
                    MessageBox.Show("Error on writing output video!");
                else
                {
                    btnSelect.Text = "처리하는 중...";
                    btnSelect.Refresh();
                    btnSelect.Enabled = false;
                    Thread thread = new Thread(onThreadProcessingVideo);
                    thread.Start();
                    //MethodInvoker mi = delegate ()
                    //{
                    //    btnSelect.Text = "Loading...";
                    //    while (IsRun() == true) ;
                    //    btnSelect.Text = "비디오를 선택합니다\n(클릭 또는 끌어서 놓기)";

                    //};
                    //this.Invoke(mi);
                    //Thread thread = 
                }

            }

        }

        private void onThreadProcessingVideo()
        {
            while(true)
            {
                if (IsRun() == false)
                    break;
                Thread.Sleep(100);
            }
            MethodInvoker mi = delegate ()
            {
                MessageBox.Show("Finished!");
                btnSelect.Text = "비디오를 선택합니다\n(클릭 또는 끌어서 놓기)";
                btnSelect.Refresh();
                btnSelect.Enabled = true;
            };
            this.Invoke(mi);
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
        }

        private void Form1_FormClosed(object sender, FormClosedEventArgs e)
        {
            Release();
            System.Diagnostics.Process.GetCurrentProcess().Kill();
        }
    }
}
