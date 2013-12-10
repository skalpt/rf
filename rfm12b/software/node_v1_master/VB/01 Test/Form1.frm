VERSION 5.00
Object = "{648A5603-2C6E-101B-82B6-000000000014}#1.1#0"; "mscomm32.ocx"
Begin VB.Form Form1 
   Caption         =   "Current Temperature"
   ClientHeight    =   495
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6975
   LinkTopic       =   "Form1"
   ScaleHeight     =   495
   ScaleWidth      =   6975
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer Timer1 
      Interval        =   100
      Left            =   6360
      Top             =   840
   End
   Begin MSCommLib.MSComm MSComm1 
      Left            =   6240
      Top             =   120
      _ExtentX        =   1005
      _ExtentY        =   1005
      _Version        =   393216
      DTREnable       =   -1  'True
   End
   Begin VB.Label Label1 
      Caption         =   "Connecting..."
      Height          =   255
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   6735
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Dim buf As String
Dim node As Integer
Dim search As String

Private Sub Form_Load()
    Me.Show
    MSComm1.CommPort = 13
    On Error GoTo ErrorHandler
    MSComm1.PortOpen = True
    On Error GoTo 0
    Label1.Caption = "Waiting..."
    node = 3
    search = "R" & node & ":"
Exit Sub
ErrorHandler:
    DoEvents
    On Error GoTo ErrorHandler
    Resume
End Sub

Private Sub Form_Unload(Cancel As Integer)
    End
End Sub

Private Sub Timer1_Timer()
    If MSComm1.InBufferCount > 0 Then
        buf = buf & MSComm1.Input
        If InStr(buf, search) Then
            If InStr(Mid(buf, InStr(buf, search) + Len(search)), vbCrLf) Then
                temp = Mid(buf, InStr(buf, search), InStr(Mid(buf, InStr(buf, search) + Len(search)), vbCrLf) + Len(search) - 1)
                Label1.Caption = temp
                buf = ""
            End If
        End If
    End If
End Sub
