VERSION 5.00
Object = "{3B7C8863-D78F-101B-B9B5-04021C009402}#1.2#0"; "RICHTX32.OCX"
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form frmEdit 
   Caption         =   "Form1"
   ClientHeight    =   7080
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   11070
   LinkTopic       =   "Form1"
   ScaleHeight     =   7080
   ScaleWidth      =   11070
   StartUpPosition =   3  'Windows Default
   Begin MSComctlLib.ProgressBar pbrUpload 
      Height          =   375
      Left            =   360
      TabIndex        =   3
      Top             =   6600
      Width           =   6615
      _ExtentX        =   11668
      _ExtentY        =   661
      _Version        =   393216
      Appearance      =   1
   End
   Begin VB.CommandButton cmdCancel 
      Caption         =   "Cancel"
      Height          =   375
      Left            =   9240
      TabIndex        =   2
      Top             =   6600
      Width           =   1455
   End
   Begin VB.CommandButton cmdSubmit 
      Caption         =   "Submit"
      Height          =   375
      Left            =   7440
      TabIndex        =   1
      Top             =   6600
      Width           =   1575
   End
   Begin RichTextLib.RichTextBox rtbCode 
      Height          =   6495
      Left            =   0
      TabIndex        =   0
      Top             =   0
      Width           =   11055
      _ExtentX        =   19500
      _ExtentY        =   11456
      _Version        =   393217
      ScrollBars      =   2
      TextRTF         =   $"frmEdit.frx":0000
   End
End
Attribute VB_Name = "frmEdit"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit
Public mush As Object
Public myobject As String
Public attr As String
Private Declare Sub Sleep Lib "kernel32" (ByVal dwMilliseconds As Long)


Private Sub cmdCancel_Click()
    Unload Me
End Sub


Private Sub cmdSubmit_Click()
    Dim counter As Long
    
    rtbCode.Text = Replace(rtbCode.Text, vbCrLf, "")
    rtbCode.Text = Replace(rtbCode.Text, vbTab, "")
    
    If Len(rtbCode.Text) = 0 Then
        MsgBox "You have nothing to submit.", vbCritical
        Exit Sub
    End If
    
    Call mush.SendData("@" & attr & " " & myobject & " = " & Mid(rtbCode.Text, 1, 1))
    
    pbrUpload.Max = Len(rtbCode.Text)
    
    mush.ignore = rtbCode.Text
    For counter = 2 To Len(rtbCode.Text)
        Select Case Mid(rtbCode.Text, counter, 1)
            Case " "
                Call mush.SendData("@edit " & myobject & "/" & attr & "=+,SPACES(1)")
            Case "{", ","
                Call mush.SendData("@edit " & myobject & "/" & attr & "=+,{" & Mid(rtbCode.Text, counter, 1) & "}")
            Case Else
                Call mush.SendData("@edit " & myobject & "/" & attr & "=+," & Mid(rtbCode.Text, counter, 1))
        End Select
        
        pbrUpload.Value = counter
        DoEvents
        Sleep 10
    Next counter
    
    Unload Me
End Sub

Private Sub Form_Load()
    Set mush.reroute = Me
    Call mush.SendData("EXAMINE " & myobject & "/" & attr)
End Sub

Public Sub rerouted(incoming As String)
    Me.Caption = "EDITING " & attr & " ATTRIBUTE OF " & myobject
    rtbCode.Text = format(incoming)
End Sub

Private Function format(ByVal code As String) As String
    Dim indent As Long
    Dim count As Long
    Dim marker As Long
    
    marker = InStr(code, ":")
    code = Mid(code, marker + 2)
    If Left(code, 1) = "$" Then
        marker = InStr(code, ":")
        indent = indent + 1
        code = Left(code, marker) & vbCrLf & tabs(indent) & Mid(code, marker + 1)
        marker = InStr(code, ":")
    Else
        marker = 1
    End If
    
    
    count = marker
    Do While count <= Len(code)
        Select Case Mid(code, count, 1)
            Case "{"
                indent = indent + 1
                code = Left(code, count - 1) & vbCrLf & tabs(indent - 1) & "{" & vbCrLf & tabs(indent) & Mid(code, count + 1)
                count = count + indent + 2
            Case "}"
                indent = indent - 1
                If indent < 0 Then indent = 0
                code = Left(code, count - 1) & vbCrLf & tabs(indent) & "}" & Mid(code, count + 1)
                count = count + indent + 2
            Case ";"
                code = Left(code, count) & vbCrLf & tabs(indent) & Mid(code, count + 1)
        End Select
        count = count + 1
    Loop
    
    
    format = code
End Function

Private Function tabs(count As Long) As String
    Dim counter As Long
    
    For counter = 1 To count
        tabs = tabs & vbTab
    Next
End Function
