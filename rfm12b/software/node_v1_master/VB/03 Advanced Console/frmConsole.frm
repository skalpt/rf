VERSION 5.00
Object = "{3B7C8863-D78F-101B-B9B5-04021C009402}#1.2#0"; "RICHTX32.OCX"
Object = "{648A5603-2C6E-101B-82B6-000000000014}#1.1#0"; "mscomm32.ocx"
Begin VB.Form frmConsole 
   Caption         =   "Communications Console"
   ClientHeight    =   4680
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   6960
   LinkTopic       =   "Form1"
   ScaleHeight     =   4680
   ScaleWidth      =   6960
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrConnect 
      Enabled         =   0   'False
      Interval        =   1
      Left            =   6360
      Top             =   840
   End
   Begin VB.Timer tmrListen 
      Enabled         =   0   'False
      Interval        =   10
      Left            =   6360
      Top             =   1440
   End
   Begin MSCommLib.MSComm MSComm 
      Left            =   6240
      Top             =   120
      _ExtentX        =   1005
      _ExtentY        =   1005
      _Version        =   393216
      DTREnable       =   -1  'True
      BaudRate        =   57600
   End
   Begin RichTextLib.RichTextBox txtConsole 
      Height          =   4455
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   6735
      _ExtentX        =   11880
      _ExtentY        =   7858
      _Version        =   393217
      BorderStyle     =   0
      ReadOnly        =   -1  'True
      ScrollBars      =   2
      TextRTF         =   $"frmConsole.frx":0000
   End
End
Attribute VB_Name = "frmConsole"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim i As Double

Private Sub Form_Load()
    
    MSComm.CommPort = 6
    MSComm.Settings = "57600,n,8,1"
    
    Me.Show
    
    MSCommConnect

End Sub

Public Sub MSCommConnect()
    
    If MSComm.PortOpen Then MSComm.PortOpen = False
    If Len(txtConsole.Text) > 0 Then addText vbCrLf
    addText "Waiting for connection..."
    
    tmrConnect.Enabled = True

End Sub

Private Sub Form_Resize()

    txtConsole.Height = Me.ScaleHeight - 240
    txtConsole.Width = Me.ScaleWidth - 240

End Sub

Private Sub Form_Unload(Cancel As Integer)

    Cancel = True
    Me.Hide

End Sub

Private Sub tmrConnect_Timer()

For i = 1 To 30
    addText "S" & i & ":" & vbCrLf
Next i

Exit Sub

    On Error GoTo ExitSub
    MSComm.PortOpen = True
    On Error GoTo 0
    tmrConnect.Enabled = False
    addText "connected." & vbCrLf
    tmrListen.Enabled = True

ExitSub:
End Sub

Private Sub tmrListen_Timer()
    
    On Error GoTo ErrorHandler
    If MSComm.InBufferCount > 0 Then
        addText Replace(MSComm.Input, Chr(10), vbCrLf)
        On Error GoTo 0
    End If
    On Error GoTo 0
Exit Sub

ErrorHandler:
    tmrListen.Enabled = False
    MSCommConnect
End Sub

Private Sub addText(addString As String)
    
    txtConsole.SelStart = Len(txtConsole.Text)
    txtConsole.SelText = addString

End Sub
