VERSION 5.00
Begin VB.Form frmNodeStatus 
   BorderStyle     =   3  'Fixed Dialog
   Caption         =   "Node Status Monitor"
   ClientHeight    =   2940
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   8040
   KeyPreview      =   -1  'True
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   2940
   ScaleWidth      =   8040
   ShowInTaskbar   =   0   'False
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer tmrWatch 
      Interval        =   100
      Left            =   7440
      Top             =   120
   End
   Begin VB.HScrollBar HScroll 
      Height          =   255
      Left            =   0
      Max             =   0
      TabIndex        =   0
      Top             =   1800
      Width           =   5775
   End
   Begin VB.VScrollBar VScroll 
      Height          =   615
      Left            =   7080
      Max             =   0
      TabIndex        =   1
      Top             =   0
      Width           =   255
   End
   Begin VB.PictureBox picCorner 
      BorderStyle     =   0  'None
      Height          =   255
      Left            =   7080
      ScaleHeight     =   255
      ScaleWidth      =   255
      TabIndex        =   2
      Top             =   600
      Width           =   255
   End
   Begin VB.PictureBox picContainer 
      BackColor       =   &H000000FF&
      BorderStyle     =   0  'None
      Height          =   735
      Left            =   0
      ScaleHeight     =   735
      ScaleWidth      =   6615
      TabIndex        =   3
      Top             =   600
      Visible         =   0   'False
      Width           =   6615
      Begin VB.PictureBox picRow 
         BackColor       =   &H000000FF&
         BorderStyle     =   0  'None
         Height          =   375
         Index           =   0
         Left            =   0
         ScaleHeight     =   375
         ScaleWidth      =   6615
         TabIndex        =   4
         Top             =   0
         Visible         =   0   'False
         Width           =   6615
         Begin VB.PictureBox picID 
            Height          =   375
            Index           =   0
            Left            =   0
            ScaleHeight     =   315
            ScaleWidth      =   435
            TabIndex        =   14
            Top             =   0
            Width           =   495
            Begin VB.Label itmID 
               Alignment       =   2  'Center
               Caption         =   "0"
               Height          =   255
               Index           =   0
               Left            =   0
               TabIndex        =   15
               Top             =   60
               Width           =   495
            End
         End
         Begin VB.PictureBox picStatus 
            Height          =   375
            Index           =   0
            Left            =   480
            ScaleHeight     =   315
            ScaleWidth      =   555
            TabIndex        =   13
            Top             =   0
            Width           =   615
            Begin VB.Shape itmStatus 
               FillColor       =   &H00FFFFFF&
               FillStyle       =   0  'Solid
               Height          =   200
               Index           =   0
               Left            =   0
               Shape           =   3  'Circle
               Top             =   60
               Width           =   615
            End
         End
         Begin VB.PictureBox picType 
            Height          =   375
            Index           =   0
            Left            =   1080
            ScaleHeight     =   315
            ScaleWidth      =   435
            TabIndex        =   11
            Top             =   0
            Width           =   495
            Begin VB.Label itmType 
               Alignment       =   2  'Center
               Caption         =   "-"
               Height          =   255
               Index           =   0
               Left            =   0
               TabIndex        =   12
               Top             =   60
               Width           =   495
            End
         End
         Begin VB.PictureBox picName 
            Height          =   375
            Index           =   0
            Left            =   1560
            ScaleHeight     =   315
            ScaleWidth      =   2115
            TabIndex        =   9
            Top             =   0
            Width           =   2175
            Begin VB.Label itmName 
               Alignment       =   2  'Center
               Caption         =   "-"
               Height          =   255
               Index           =   0
               Left            =   0
               TabIndex        =   10
               Top             =   60
               Width           =   2175
            End
         End
         Begin VB.PictureBox picReading 
            Height          =   375
            Index           =   0
            Left            =   3720
            ScaleHeight     =   315
            ScaleWidth      =   795
            TabIndex        =   7
            Top             =   0
            Width           =   855
            Begin VB.Label itmReading 
               Alignment       =   2  'Center
               Caption         =   "-"
               Height          =   255
               Index           =   0
               Left            =   0
               TabIndex        =   8
               Top             =   60
               Width           =   855
            End
         End
         Begin VB.PictureBox picTimestamp 
            Height          =   375
            Index           =   0
            Left            =   4560
            ScaleHeight     =   315
            ScaleWidth      =   1995
            TabIndex        =   5
            Top             =   0
            Width           =   2055
            Begin VB.Label itmTimestamp 
               Alignment       =   2  'Center
               Caption         =   "-"
               Height          =   255
               Index           =   0
               Left            =   0
               TabIndex        =   6
               Top             =   60
               Width           =   2055
            End
         End
      End
   End
   Begin VB.PictureBox picHeader 
      BackColor       =   &H000000FF&
      BorderStyle     =   0  'None
      Height          =   615
      Left            =   0
      ScaleHeight     =   615
      ScaleWidth      =   6615
      TabIndex        =   16
      Top             =   0
      Width           =   6615
      Begin VB.Label lblTimestamp 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "Timestamp"
         Height          =   255
         Left            =   4560
         TabIndex        =   17
         Top             =   0
         Width           =   2055
      End
      Begin VB.Label lblReading 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "Reading"
         Height          =   255
         Left            =   3720
         TabIndex        =   18
         Top             =   0
         Width           =   855
      End
      Begin VB.Label lblName 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "Name"
         Height          =   255
         Left            =   1560
         TabIndex        =   19
         Top             =   0
         Width           =   2175
      End
      Begin VB.Label lblType 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "Type"
         Height          =   255
         Left            =   1080
         TabIndex        =   20
         Top             =   0
         Width           =   495
      End
      Begin VB.Label lblStatus 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "Status"
         Height          =   255
         Left            =   480
         TabIndex        =   21
         Top             =   0
         Width           =   615
      End
      Begin VB.Label lblID 
         Alignment       =   2  'Center
         BorderStyle     =   1  'Fixed Single
         Caption         =   "ID"
         Height          =   255
         Left            =   0
         TabIndex        =   22
         Top             =   0
         Width           =   495
      End
   End
End
Attribute VB_Name = "frmNodeStatus"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Const SPI_GETWORKAREA = 48

Private Declare Function SysParametersInfo& Lib "User32" Alias "SystemParametersInfoA" ( _
    ByVal uAction As Long, _
    ByVal uParam As Long, _
    lvpParam As Any, _
    ByVal fuWinIni As Long)

Private Type RECT
    Left As Long
    Top As Long
    Right As Long
    Bottom As Long
End Type

Dim ScreenHeight As Integer, ScreenWidth, ScreenLeft As Integer, ScreenTop As Integer

Const numElements = 31
Dim i As Integer
Dim consoleCursor As Double
Dim pollID As Integer, lastPollID As Integer

Private Sub Form_KeyDown(KeyCode As Integer, Shift As Integer)

    If Me.ActiveControl Is VScroll Or Me.ActiveControl Is HScroll Then
    Else
        Select Case KeyCode
            Case 37
                If HScroll.Value > HScroll.Min Then HScroll.Value = HScroll.Value - 1
            Case 38
                If VScroll.Value > VScroll.Min Then VScroll.Value = VScroll.Value - 1
            Case 39
                If HScroll.Value < HScroll.Max Then HScroll.Value = HScroll.Value + 1
            Case 40
                If VScroll.Value < VScroll.Max Then VScroll.Value = VScroll.Value + 1
        End Select
    End If
    
End Sub

Private Sub Form_Load()

    consoleCursor = 1

    PositionContainers
    GetScreenHeight
    LoadElements
    ResizeForm
    Me.Show

    Load frmConsole

End Sub

Private Sub PositionContainers()

    picHeader.Top = 0
    picHeader.Height = lblID.Height
    picContainer.Top = picHeader.Height

End Sub

Private Sub GetScreenHeight()
    
    Dim DesktopArea As RECT
    Call SysParametersInfo(SPI_GETWORKAREA, 0, DesktopArea, 0)
    
    ScreenHeight = (DesktopArea.Bottom - DesktopArea.Top) * Screen.TwipsPerPixelY
    ScreenWidth = (DesktopArea.Right - DesktopArea.Left) * Screen.TwipsPerPixelX
    ScreenLeft = DesktopArea.Left * Screen.TwipsPerPixelX
    ScreenTop = DesktopArea.Top * Screen.TwipsPerPixelY

End Sub

Private Sub LoadElements()
    
    For i = 1 To numElements
                
        Load picRow(i)
        picRow(i).Top = picRow(0).Top + picRow(0).Height * (i - 1)
        HScroll.Top = HScroll.Top + picRow(i).Height
        VScroll.Height = VScroll.Height + picRow(i).Height
        
        LoadElement picID, itmID
        itmID(i).Caption = i
        LoadElement picStatus, itmStatus
        LoadElement picType, itmType
        LoadElement picName, itmName
        LoadElement picReading, itmReading
        LoadElement picTimestamp, itmTimestamp
        
        picRow(i).Visible = True
    
    Next i
    
    picContainer.Width = picRow(0).Width
    picContainer.Height = picRow(picRow.Ubound).Top + picRow(picRow.Ubound).Height
    picContainer.Visible = True

End Sub

Private Sub LoadElement(Parent As Object, Child As Object)

    Load Parent(i)
    Set Parent(i).Container = picRow(i)
    Parent(i).Visible = True
    Load Child(i)
    Set Child(i).Container = Parent(i)
    Child(i).Visible = True

End Sub

Private Sub ResizeForm()
   
    ResizeFormHeight
    ResizeFormWidth

    Me.Move (ScreenWidth - Me.Width) / 2 + ScreenLeft, (ScreenHeight - Me.Height) / 2 + ScreenTop

End Sub

Private Sub ResizeFormHeight()

    If Me.Height - Me.ScaleHeight + picContainer.Top + picContainer.Height + HScroll.Height > ScreenHeight Then
        Me.Height = ScreenHeight
    Else
        Me.Height = Me.Height - Me.ScaleHeight + picContainer.Top + picContainer.Height + HScroll.Height
    End If

End Sub

Private Sub ResizeFormWidth()

    If Me.Width - Me.ScaleWidth + picContainer.Width + VScroll.Width > ScreenWidth Then
        Me.Width = ScreenWidth
    Else
        Me.Width = Me.Width - Me.ScaleWidth + picContainer.Width + VScroll.Width
    End If

End Sub

Private Sub Form_Resize()

    If Me.ScaleHeight - VScroll.Top - HScroll.Height <= 0 Then Exit Sub
    If Me.ScaleWidth - VScroll.Width <= 0 Then Exit Sub
    If picHeader.Height + picContainer.Height + HScroll.Height < Me.ScaleHeight Then ResizeFormHeight
    If picContainer.Width + VScroll.Width < Me.ScaleWidth Then ResizeFormWidth

    HScroll.Top = Me.ScaleHeight - HScroll.Height
    VScroll.Height = Me.ScaleHeight - VScroll.Top - HScroll.Height
    If HScroll.Top < picHeader.Height + picContainer.Height Then
        VScrollEnabled True
    Else
        VScrollEnabled False
    End If

    VScroll.Left = Me.ScaleWidth - VScroll.Width
    HScroll.Width = Me.ScaleWidth - VScroll.Width
    If VScroll.Left < picContainer.Width Then
        HScrollEnabled True
    Else
        HScrollEnabled False
    End If
    
    picCorner.Top = HScroll.Top
    picCorner.Left = VScroll.Left

End Sub

Private Sub VScrollEnabled(isEnabled As Boolean)
    
    If isEnabled Then
        If Me.ActiveControl Is VScroll Then picCorner.SetFocus
        VScroll.Max = picRow.Count - 1 - Round((HScroll.Top - picHeader.Height) / picRow(0).Height, 0)
        Call VScroll_Change
        VScroll.Enabled = True
    Else
        VScroll.Enabled = False
        VScroll.Max = 0
    End If

End Sub

Private Sub HScrollEnabled(isEnabled As Boolean)

    If isEnabled Then
        If Me.ActiveControl Is HScroll Then picCorner.SetFocus
        HScroll.Max = (picContainer.Width - VScroll.Left) / picRow(0).Height
        If HScroll.Max = 0 Then HScroll.Max = 1
        Call HScroll_Change
        HScroll.Enabled = True
    Else
        HScroll.Enabled = False
        HScroll.Max = 0
    End If
    
End Sub

Private Sub Form_Unload(Cancel As Integer)
    
    End

End Sub

Private Sub tmrWatch_Timer()

    Dim processText As String

    If InStr(consoleCursor, frmConsole.txtConsole.Text, vbCrLf) Then
        Do While InStr(InStr(consoleCursor, frmConsole.txtConsole.Text, vbCrLf) + 2, frmConsole.txtConsole.Text, vbCrLf)
            processText = Mid(frmConsole.txtConsole.Text, InStr(consoleCursor, frmConsole.txtConsole.Text, vbCrLf) + 2, InStr(InStr(consoleCursor, frmConsole.txtConsole.Text, vbCrLf) + 2, frmConsole.txtConsole.Text, vbCrLf) - InStr(consoleCursor, frmConsole.txtConsole.Text, vbCrLf) - 2)
            If Left(processText, 1) = "S" And InStr(processText, ":") Then
                lastPollID = pollID
                pollID = Val(Mid(processText, 2, InStr(processText, ":") - 2))
                If lastPollID <> pollID And itmStatus(lastPollID).FillColor = &HFFFF& Then
                    itmStatus(lastPollID).FillColor = &HFF&
                End If
                itmStatus(pollID).FillColor = &HFFFF&
            End If
            If Left(processText, 1) = "R" And InStr(processText, ":") Then
                itmStatus(pollID).FillColor = &HFF00&
                processText = Mid(processText, InStr(processText, ":") + 1)
                Select Case Left(processText, 1)
                    Case "I"
                        If Mid(processText, 2, 1) = "" Then
                            itmType(pollID) = "-"
                        Else
                            itmType(pollID) = Mid(processText, 2, 1)
                        End If
                        If Mid(processText, 3) = "" Then
                            itmName(pollID) = "-"
                        Else
                            itmName(pollID) = Mid(processText, 3)
                        End If
                    Case "R"
                        If InStr(processText, ",") Then
                            'Don't care about this first one, it's just a unique ID:
                            processText = Mid(processText, InStr(processText, ",") + 1)
                            If InStr(processText, ",") Then
                                Dim reading, readingOffset
                                reading = Left(processText, InStr(processText, ",") - 1)
                                readingOffset = Mid(processText, InStr(processText, ",") + 1)
                                Select Case itmType(pollID)
                                    Case "T"
                                        itmReading(pollID) = Round(Val(reading) * 0.0625, 2) & Chr(176) & "C"
                                        If InStrRev(readingOffset, ":") Then
                                            readingOffset = Val(Mid(readingOffset, InStrRev(readingOffset, ":") + 1)) / 1000
                                        Else
                                            readingOffset = Val(readingOffset) / 1000
                                        End If
                                        itmTimestamp(pollID) = Format(DateAdd("s", -readingOffset, Now), "dd/mm/yyyy hh:mm:ss")
                                End Select
                            End If
                        End If
                End Select
            End If
            DoEvents
            consoleCursor = InStr(InStr(consoleCursor, frmConsole.txtConsole.Text, vbCrLf) + 2, frmConsole.txtConsole.Text, vbCrLf)
        Loop
    End If

End Sub

Private Sub VScroll_Change()

    If VScroll.Value = VScroll.Max Then
        picContainer.Top = HScroll.Top - picContainer.Height
    Else
        picContainer.Top = picHeader.Height - (VScroll.Value * picRow(0).Height)
    End If

End Sub

Private Sub VScroll_Scroll()

    Call VScroll_Change
    
End Sub

Private Sub HScroll_Change()

    If HScroll.Value = HScroll.Max Then
        picContainer.Left = VScroll.Left - picContainer.Width
        picHeader.Left = VScroll.Left - picContainer.Width
    Else
        picContainer.Left = -HScroll.Value * picRow(0).Height
        picHeader.Left = -HScroll.Value * picRow(0).Height
    End If

End Sub

Private Sub HScroll_Scroll()

    Call HScroll_Change
    
End Sub
