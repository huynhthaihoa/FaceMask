﻿using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using System.Threading;

namespace FaceMask
{
    public partial class Form1 : Form
    {

        public delegate void UpdateStatusCallback(long hour, long minute, long second);

        [DllImport("FFMPegModule.dll", EntryPoint = "IsRun", CallingConvention = CallingConvention.Cdecl)]
        private static extern bool IsRun();

        [DllImport("FFMPegModule.dll", EntryPoint = "Process", CallingConvention = CallingConvention.Cdecl)]
        private static extern int Process(string strInputFile, string strOutputFile, long bitrate, float fps, long duration);

        [DllImport("FFMPegModule.dll", EntryPoint = "Release", CallingConvention = CallingConvention.Cdecl)]
        private static extern void Release();

        [DllImport("FFMPegModule.dll", EntryPoint = "SetUpdateStatusCallback", CallingConvention = CallingConvention.Cdecl)]
        private static extern void SetUpdateStatusCallback(UpdateStatusCallback callback);

        //private int status;
        //[DllImport("FFMPegModule.dll", EntryPoint = "Test", CallingConvention = CallingConvention.Cdecl)]
        //private static extern int Test();

        //private string _inputFileName;

        //private string _outputFileName;
        //long mBitRate;

        //float mFPS;

        long mBitRate;

        float mFPS;

        long mDuration;

        public Form1()
        {
            InitializeComponent();

            mBitRate = 6000000;
            mFPS = 30;
            mDuration = 0;

            cbEnableSetting_CheckedChanged(null, null);
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
            long bitrate = long.Parse(tbBitRate.Text);
            float fps = float.Parse(tbFPS.Text);
            long duration = long.Parse(tbDuration.Text);

            MessageBox.Show("파일 저장 대상을 선택하십시오.");
            SaveFileDialog dlg = new SaveFileDialog();
            string ext = Path.GetExtension(srcPath);
            dlg.FileName = Path.GetFileNameWithoutExtension(srcPath) + "_output";
            //dlg.DefaultExt = ext;
            //dlg.Filter = "MP4 비디오|*.mp4|AVI 비디오|*.avi";
            dlg.Filter = "H264 비디오|*.h264|MP4 비디오|*.mp4|AVI 비디오|*.avi";
            if (dlg.ShowDialog() == DialogResult.OK)
            {
                string dstPath = dlg.FileName;
                int ret = Process(srcPath, dstPath, bitrate, fps, duration);
                if (ret < 0)
                    MessageBox.Show("입력 비디오를 읽는 동안 오류가 발생했습니다!");
                else if (ret > 0)
                    MessageBox.Show("출력 비디오를 쓰는 동안 오류가 발생했습니다!");
                else
                {
                    btnSelect.Text = "처리하는 중...";
                    btnSelect.Refresh();
                    btnSelect.Enabled = false;
                    //UpdateStatus(delegate(long msg) {
                    //    MessageBox.Show("이미 " + msg.ToString() + "초 처리됨!");
                    //});
                    SetUpdateStatusCallback(delegate (long hour, long minute, long second)
                    {
                        MethodInvoker mi = delegate ()
                        {
                            string msg = "이미 ";
                            if (hour > 0)
                                msg += hour.ToString() + "시간 ";
                            if (minute > 0)
                                msg += minute.ToString() + "분 ";
                            if (second > 0)
                                msg += second.ToString() + "초 ";
                            msg += "처리되었습니다! 처리하는 중...";

                            //MessageBox.Show("끝났습니다!");
                            btnSelect.Text = msg;// "비디오를 선택합니다\n(클릭 또는 끌어서 놓기)";
                            btnSelect.Refresh();
                            //btnSelect.Enabled = true;
                        };
                        btnSelect.Invoke(mi);
                        //btnSelect.Invoke(delegate ()
                        //{ });
                        //btnSelect.Invoke(delegate (long hour, long minute, long second)
                        //{

                        //});
                        //string msg = "이미 ";
                        //if (hour > 0)
                        //    msg += hour.ToString() + "시간 ";
                        //if (minute > 0)
                        //    msg += minute.ToString() + "분 ";
                        //if (second > 0)
                        //    msg += second.ToString() + "초 ";
                        //msg += "처리되었습니다!";
                        //MessageBox.Show(msg);
                    });
                    Thread thread = new Thread(onThreadProcessingVideo);
                    thread.Start();
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
                MessageBox.Show("끝났습니다!");
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

        private void tbKeyPress(object sender, KeyPressEventArgs e)
        {
            e.Handled = !(char.IsNumber(e.KeyChar) || e.KeyChar == (char)Keys.Back || e.KeyChar == (char)Keys.Space);
        }

        private void cbEnableSetting_CheckedChanged(object sender, EventArgs e)
        {
            gbOutputSetting.Enabled = cbEnableSetting.Checked;

            if(gbOutputSetting.Enabled == false)
            {
                tbBitRate.Text = mBitRate.ToString();
                tbFPS.Text = mFPS.ToString();
                tbDuration.Text = mDuration.ToString();
            }
        }
    }
}
