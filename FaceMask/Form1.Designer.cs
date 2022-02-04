
namespace FaceMask
{
    partial class Form1
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.btnSelect = new System.Windows.Forms.Button();
            this.lbStatus = new System.Windows.Forms.Label();
            this.gbOutputSetting = new System.Windows.Forms.GroupBox();
            this.tbInterval = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.tbFPS = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.tbBitRate = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.cbEnableSetting = new System.Windows.Forms.CheckBox();
            this.gbOutputSetting.SuspendLayout();
            this.SuspendLayout();
            // 
            // btnSelect
            // 
            this.btnSelect.AllowDrop = true;
            this.btnSelect.BackColor = System.Drawing.Color.Beige;
            this.btnSelect.Location = new System.Drawing.Point(210, 45);
            this.btnSelect.Name = "btnSelect";
            this.btnSelect.Size = new System.Drawing.Size(555, 456);
            this.btnSelect.TabIndex = 0;
            this.btnSelect.Text = "비디오를 선택합니다\r\n(클릭 또는 끌어서 놓기)";
            this.btnSelect.UseVisualStyleBackColor = false;
            this.btnSelect.Click += new System.EventHandler(this.btnSelect_Click);
            this.btnSelect.DragDrop += new System.Windows.Forms.DragEventHandler(this.btnSelect_DragDrop);
            this.btnSelect.DragEnter += new System.Windows.Forms.DragEventHandler(this.btnSelect_DragEnter);
            // 
            // lbStatus
            // 
            this.lbStatus.AutoSize = true;
            this.lbStatus.Location = new System.Drawing.Point(334, 529);
            this.lbStatus.Name = "lbStatus";
            this.lbStatus.Size = new System.Drawing.Size(0, 32);
            this.lbStatus.TabIndex = 1;
            // 
            // gbOutputSetting
            // 
            this.gbOutputSetting.Controls.Add(this.tbInterval);
            this.gbOutputSetting.Controls.Add(this.label3);
            this.gbOutputSetting.Controls.Add(this.tbFPS);
            this.gbOutputSetting.Controls.Add(this.label2);
            this.gbOutputSetting.Controls.Add(this.tbBitRate);
            this.gbOutputSetting.Controls.Add(this.label1);
            this.gbOutputSetting.Location = new System.Drawing.Point(30, 617);
            this.gbOutputSetting.Name = "gbOutputSetting";
            this.gbOutputSetting.Size = new System.Drawing.Size(939, 196);
            this.gbOutputSetting.TabIndex = 2;
            this.gbOutputSetting.TabStop = false;
            this.gbOutputSetting.Text = "출력 설정";
            // 
            // tbInterval
            // 
            this.tbInterval.Location = new System.Drawing.Point(685, 103);
            this.tbInterval.Name = "tbInterval";
            this.tbInterval.Size = new System.Drawing.Size(209, 39);
            this.tbInterval.TabIndex = 5;
            this.tbInterval.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.tbKeyPress);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(495, 103);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(162, 32);
            this.label3.TabIndex = 4;
            this.label3.Text = "분할 간격 (초)";
            // 
            // tbFPS
            // 
            this.tbFPS.Location = new System.Drawing.Point(180, 135);
            this.tbFPS.Name = "tbFPS";
            this.tbFPS.Size = new System.Drawing.Size(209, 39);
            this.tbFPS.TabIndex = 3;
            this.tbFPS.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.tbKeyPress);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(18, 135);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(141, 32);
            this.label2.TabIndex = 2;
            this.label2.Text = "초당 프레임";
            // 
            // tbBitRate
            // 
            this.tbBitRate.Location = new System.Drawing.Point(180, 63);
            this.tbBitRate.Name = "tbBitRate";
            this.tbBitRate.Size = new System.Drawing.Size(209, 39);
            this.tbBitRate.TabIndex = 1;
            this.tbBitRate.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.tbKeyPress);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(18, 63);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(141, 32);
            this.label1.TabIndex = 0;
            this.label1.Text = "비트 전송률";
            // 
            // cbEnableSetting
            // 
            this.cbEnableSetting.AutoSize = true;
            this.cbEnableSetting.Location = new System.Drawing.Point(383, 550);
            this.cbEnableSetting.Name = "cbEnableSetting";
            this.cbEnableSetting.Size = new System.Drawing.Size(228, 36);
            this.cbEnableSetting.TabIndex = 4;
            this.cbEnableSetting.Text = "출력 설정 활성화";
            this.cbEnableSetting.UseVisualStyleBackColor = true;
            this.cbEnableSetting.CheckedChanged += new System.EventHandler(this.cbEnableSetting_CheckedChanged);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(13F, 32F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1001, 835);
            this.Controls.Add(this.cbEnableSetting);
            this.Controls.Add(this.gbOutputSetting);
            this.Controls.Add(this.lbStatus);
            this.Controls.Add(this.btnSelect);
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.Text = "FaceMask";
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Form1_FormClosed);
            this.gbOutputSetting.ResumeLayout(false);
            this.gbOutputSetting.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button btnSelect;
        private System.Windows.Forms.Label lbStatus;
        private System.Windows.Forms.GroupBox gbOutputSetting;
        private System.Windows.Forms.TextBox tbBitRate;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbFPS;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox tbInterval;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox cbEnableSetting;
    }
}

