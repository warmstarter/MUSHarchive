VERSION 5.00
Object = "{3B7C8863-D78F-101B-B9B5-04021C009402}#1.2#0"; "RICHTX32.OCX"
Begin VB.Form frmSession 
   Caption         =   "Session"
   ClientHeight    =   6510
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   10875
   LinkTopic       =   "Form1"
   MDIChild        =   -1  'True
   ScaleHeight     =   6510
   ScaleWidth      =   10875
   Begin VB.Timer Timer1 
      Interval        =   1000
      Left            =   10080
      Top             =   5040
   End
   Begin VB.TextBox txtINPUT 
      BeginProperty Font 
         Name            =   "Courier New"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   1575
      Left            =   0
      ScrollBars      =   2  'Vertical
      TabIndex        =   1
      Top             =   4920
      Width           =   10815
   End
   Begin RichTextLib.RichTextBox rtbOUTPUT 
      Height          =   4815
      Left            =   0
      TabIndex        =   0
      TabStop         =   0   'False
      Top             =   0
      Width           =   10815
      _ExtentX        =   19076
      _ExtentY        =   8493
      _Version        =   393217
      BackColor       =   0
      ReadOnly        =   -1  'True
      ScrollBars      =   2
      Appearance      =   0
      TextRTF         =   $"frmSession.frx":0000
      BeginProperty Font {0BE35203-8F91-11CE-9DE3-00AA004BB851} 
         Name            =   "Courier New"
         Size            =   9.75
         Charset         =   0
         Weight          =   400
         Underline       =   0   'False
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
   End
End
Attribute VB_Name = "frmSession"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Public WithEvents mush As mshTinyTIM
Attribute mush.VB_VarHelpID = -1
Private CommandBuffer() As String
Private bufferPosition As Long

Private Declare Function FlashWindow Lib "user32" _
  (ByVal hwnd As Long, _
   ByVal bInvert As Long) As Long
   
Private Declare Function GetActiveWindow Lib "user32" () As Long


Private Sub Form_Load()
    Set mush = New mshTinyTIM
    mush.Connect
    EnableURLDetect rtbOUTPUT.hwnd, Me.hwnd
    Me.WindowState = vbMaximized
    ReDim CommandBuffer(0)
    bufferPosition = 0
End Sub


Private Sub Form_Resize()
    rtbOUTPUT.Top = 0
    rtbOUTPUT.Left = 0
    rtbOUTPUT.Height = ScaleHeight * 0.8
    rtbOUTPUT.width = ScaleWidth
    
    txtINPUT.Top = rtbOUTPUT.Top + rtbOUTPUT.Height
    txtINPUT.Height = ScaleHeight * 0.2
    txtINPUT.Left = 0
    txtINPUT.width = ScaleWidth
End Sub


Private Sub MUSH_DataArrived(ByVal buffer As String, color As Long)
    AddLine buffer, color
End Sub



Private Sub txtINPUT_KeyDown(KeyCode As Integer, Shift As Integer)
    Select Case KeyCode
        Case vbKeyReturn
            If Left(txtINPUT.Text, 1) = "/" Then
                Call mush.commandMode(txtINPUT.Text)
                txtINPUT = ""
                txtINPUT.SelStart = 1
                Exit Sub
            End If
            
            CommandBuffer(UBound(CommandBuffer)) = txtINPUT.Text
            bufferPosition = UBound(CommandBuffer)
            ReDim Preserve CommandBuffer(UBound(CommandBuffer) + 1)
            mush.SendData txtINPUT.Text
            txtINPUT = ""
            txtINPUT.SelStart = 1
        
        Case vbKeyPageUp
            DisableURLDetect            'PROGRAM CRASHES ON PGUP WITH URLDETECT ON
            rtbOUTPUT.SetFocus
            SendKeys "{pgup}", True
            txtINPUT.SetFocus
            EnableURLDetect rtbOUTPUT.hwnd, Me.hwnd
        
        Case vbKeyPageDown
            DisableURLDetect            'PROGRAM CRASHES ON PGUP WITH URLDETECT ON
            rtbOUTPUT.SetFocus
            SendKeys "{pgdn}", True
            txtINPUT.SetFocus
            EnableURLDetect rtbOUTPUT.hwnd, Me.hwnd
        
        Case vbKeyUp
            txtINPUT.Text = CommandBuffer(bufferPosition)
            bufferPosition = bufferPosition - 1
            If bufferPosition < LBound(CommandBuffer) Then bufferPosition = LBound(CommandBuffer)
        
        Case vbKeyDown
            txtINPUT.Text = CommandBuffer(bufferPosition)
            bufferPosition = bufferPosition + 1
            If bufferPosition > UBound(CommandBuffer) Then bufferPosition = UBound(CommandBuffer)
            
    End Select
End Sub

Public Sub AddLine(ByVal strLine, ByVal lngColor As Long)
    
    Dim counter As Long
    Dim marker As Long
    Dim startout As Long
    Dim www As String
    Dim http As String
    Dim position As Long
    
    With rtbOUTPUT
        ' Set the cursor at the end.
        startout = Len(.Text)
        .SelStart = startout
        
'**************BEGIN CODE FOR WORDWRAP INDENTING*****************
        If Len(strLine) > 80 Then
                marker = 80
                position = 0
                Do While marker < Len(strLine)
                    marker = InStrRev(strLine, " ", marker)
                    If position = marker - 5 Then Exit Do
                    position = marker
                    strLine = Left$(strLine, marker) & vbCrLf & "   " & Mid(strLine, marker + 1)
                    marker = marker + 80
                Loop
            End If
            If strLine <> "" Then strLine = strLine & vbCrLf

'**************END WORDWRAP INDENT********************************
            .SelText = strLine
            .SelStart = Len(.Text)
        ' Select the new text.
        .SelStart = startout
        If .SelStart <> 0 Then .SelStart = .SelStart - 1
        .SelLength = Len(.Text)
        ' Format the new text.
        .SelColor = lngColor
    
        'RESET SELECTION SO TEST IS NOT HIGHLIGHTED
        .SelStart = Len(rtbOUTPUT.Text)
    End With
    If flashmode And strLine <> "" Then Call FlashWindow(frmMain.hwnd, True)
End Sub

