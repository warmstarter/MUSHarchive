VERSION 5.00
Begin VB.Form frmFlags 
   Caption         =   "Set Flags"
   ClientHeight    =   3555
   ClientLeft      =   60
   ClientTop       =   345
   ClientWidth     =   6540
   LinkTopic       =   "Form1"
   ScaleHeight     =   3555
   ScaleWidth      =   6540
   StartUpPosition =   1  'CenterOwner
   Tag             =   "table1.id ="
   Begin VB.CheckBox Check1 
      Caption         =   "XECUTE_OK"
      Height          =   195
      Index           =   34
      Left            =   4440
      TabIndex        =   36
      Tag             =   "x"
      Top             =   2400
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "XAMINE_OK"
      Height          =   195
      Index           =   33
      Left            =   4440
      TabIndex        =   35
      Tag             =   "X"
      Top             =   2160
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "WIZARD"
      Enabled         =   0   'False
      Height          =   195
      Index           =   32
      Left            =   4440
      TabIndex        =   34
      Tag             =   "W"
      Top             =   1920
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "TRANSPARENT"
      Height          =   195
      Index           =   31
      Left            =   4440
      TabIndex        =   33
      Tag             =   "t"
      Top             =   1680
      Width           =   2000
   End
   Begin VB.CommandButton cmdCancel 
      Cancel          =   -1  'True
      Caption         =   "CANCEL"
      Height          =   375
      Left            =   3360
      TabIndex        =   32
      Top             =   3120
      Width           =   1575
   End
   Begin VB.CommandButton cmdSubmit 
      Caption         =   "SUBMIT"
      Height          =   375
      Left            =   1560
      TabIndex        =   31
      Top             =   3120
      Width           =   1575
   End
   Begin VB.CheckBox Check1 
      Caption         =   "TERSE"
      Height          =   195
      Index           =   30
      Left            =   4440
      TabIndex        =   30
      Tag             =   "T"
      Top             =   1440
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "TEMPLE"
      Enabled         =   0   'False
      Height          =   195
      Index           =   29
      Left            =   4440
      TabIndex        =   29
      Tag             =   "T"
      Top             =   1200
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "STICKY"
      Height          =   195
      Index           =   28
      Left            =   4440
      TabIndex        =   28
      Tag             =   "S"
      Top             =   960
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "SILENT"
      Height          =   195
      Index           =   27
      Left            =   4440
      TabIndex        =   27
      Tag             =   "Q"
      Top             =   720
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "SLAVE"
      Enabled         =   0   'False
      Height          =   195
      Index           =   26
      Left            =   4440
      TabIndex        =   26
      Tag             =   "s"
      Top             =   480
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "SEE_SLEEPERS"
      Height          =   195
      Index           =   25
      Left            =   4440
      TabIndex        =   25
      Tag             =   "Z"
      Top             =   240
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "ROOM"
      Enabled         =   0   'False
      Height          =   195
      Index           =   24
      Left            =   4440
      TabIndex        =   24
      Tag             =   "R"
      Top             =   0
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "QUIET"
      Height          =   195
      Index           =   23
      Left            =   2280
      TabIndex        =   23
      Tag             =   "q"
      Top             =   2760
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "PUPPET"
      Height          =   195
      Index           =   22
      Left            =   2280
      TabIndex        =   22
      Tag             =   "p"
      Top             =   2520
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "PLAYER"
      Enabled         =   0   'False
      Height          =   195
      Index           =   21
      Left            =   2280
      TabIndex        =   21
      Tag             =   "P"
      Top             =   2280
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "OPEN"
      Height          =   195
      Index           =   20
      Left            =   2280
      TabIndex        =   20
      Tag             =   "o"
      Top             =   2040
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "OPAQUE"
      Height          =   195
      Index           =   19
      Left            =   2280
      TabIndex        =   19
      Tag             =   "O"
      Top             =   1800
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "NO_EMIT"
      Height          =   195
      Index           =   18
      Left            =   2280
      TabIndex        =   18
      Tag             =   "N"
      Top             =   1560
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "NEARSIGHTED"
      Height          =   195
      Index           =   17
      Left            =   2280
      TabIndex        =   17
      Tag             =   "n"
      Top             =   1320
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "MARKED"
      Height          =   195
      Index           =   16
      Left            =   2280
      TabIndex        =   16
      Tag             =   "M"
      Top             =   1080
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "LINK_OK"
      Height          =   195
      Index           =   15
      Left            =   2280
      TabIndex        =   15
      Tag             =   "L"
      Top             =   840
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "LIGHT"
      Height          =   195
      Index           =   14
      Left            =   2280
      TabIndex        =   14
      Tag             =   "l"
      Top             =   600
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "KEY"
      Height          =   195
      Index           =   13
      Left            =   2280
      TabIndex        =   13
      Tag             =   "K"
      Top             =   360
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "INSOMNIAC"
      Height          =   195
      Index           =   12
      Left            =   2280
      TabIndex        =   12
      Tag             =   "z"
      Top             =   120
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "INDESTRUCTABLE"
      Height          =   195
      Index           =   11
      Left            =   120
      TabIndex        =   11
      Tag             =   "I"
      Top             =   2760
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "HAVEN"
      Height          =   195
      Index           =   10
      Left            =   120
      TabIndex        =   10
      Tag             =   "H"
      Top             =   2520
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "HALT"
      Height          =   195
      Index           =   9
      Left            =   120
      TabIndex        =   9
      Tag             =   "H"
      Top             =   2280
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "GOING"
      Height          =   195
      Index           =   8
      Left            =   120
      TabIndex        =   8
      Tag             =   "G"
      Top             =   2040
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "FLOATING"
      Enabled         =   0   'False
      Height          =   195
      Index           =   7
      Left            =   120
      TabIndex        =   7
      Tag             =   "F"
      Top             =   1800
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "EXIT"
      Enabled         =   0   'False
      Height          =   195
      Index           =   6
      Left            =   120
      TabIndex        =   6
      Tag             =   "E"
      Top             =   1560
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "ENTER_OK"
      Height          =   195
      Index           =   5
      Left            =   120
      TabIndex        =   5
      Tag             =   "e"
      Top             =   1320
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "DESTROY_OK"
      Height          =   195
      Index           =   4
      Left            =   120
      TabIndex        =   4
      Tag             =   "d"
      Top             =   1080
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "DARK"
      Height          =   195
      Index           =   3
      Left            =   120
      TabIndex        =   3
      Tag             =   "D"
      Top             =   840
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "CONNECTED"
      Enabled         =   0   'False
      Height          =   195
      Index           =   2
      Left            =   120
      TabIndex        =   2
      Tag             =   "c"
      Top             =   600
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "CHOWN_OK"
      Height          =   195
      Index           =   1
      Left            =   120
      TabIndex        =   1
      Tag             =   "C"
      Top             =   360
      Width           =   2000
   End
   Begin VB.CheckBox Check1 
      Caption         =   "ABODE"
      Height          =   195
      Index           =   0
      Left            =   120
      TabIndex        =   0
      Tag             =   "A"
      Top             =   120
      Width           =   2000
   End
End
Attribute VB_Name = "frmFlags"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Public Event systemmessage(message As String)

Public mush As Object
Public myobject As String
Private changed() As Boolean


Private Sub Check1_Click(Index As Integer)
    If changed(Index) Then
        changed(Index) = False
    Else
        changed(Index) = True
    End If
End Sub

Private Sub cmdCancel_Click()
    Unload Me
End Sub

Private Sub cmdSubmit_Click()
    Dim counter As Long
    Dim strTemp As String
    
    For counter = Check1.LBound To Check1.UBound
        If changed(counter) Then
            strTemp = ""
            strTemp = "@set " & myobject & " = "
            If Check1(counter).Value = 0 Then strTemp = strTemp & "!"
            strTemp = strTemp & Check1(counter).Caption
            mush.SendData (strTemp)
            mush.systemmessage strTemp
        End If
    Next
    Unload Me
End Sub

Private Sub Form_Load()
    Me.Caption = "SET FLAGS FOR " & myobject
    ReDim changed(Check1.UBound)
    Set mush.reroute = Me
    Call mush.SendData("think if(eq(OWNER(" & _
        myobject & "),NUM(ME)),IF(eq(flags(" & myobject & _
        "),{}),NO FLAGS,flags(" & myobject & ")),NOT OWNER)")
End Sub

Public Sub rerouted(incoming As String)
    Dim counter As Long
    
    If incoming = "I don't see that here." Then
            MsgBox "No such object"
            Exit Sub
    End If
    
    incoming = Mid(incoming, 24)
    incoming = Replace(incoming, Chr(34), "")
    
    Select Case incoming
        Case "I don't see that here."
            MsgBox "No such object"
        Case "NO FLAGS"
            ReDim changed(Check1.UBound)
            Exit Sub
        Case "NOT OWNER"
            MsgBox "You cannot edit the flags of this object", vbCritical, "Access Denied"
            Unload Me
        Case Else
            For counter = Check1.LBound To Check1.UBound
                If InStr(incoming, Check1(counter).Tag) Then
                    Check1(counter).Value = 1
                End If
            Next counter
    End Select
    cleanFlags
    ReDim changed(Check1.UBound)
End Sub

Private Sub cleanFlags()
    Dim counter As Long
    'YOU CAN ONLY UNCHECK THE GOING FLAG
    If Check1(8).Value <> 1 Then Check1(8).Enabled = False
    'DISABLE CHECK NOT AVAILABLE TO EXITS
    If Check1(6).Value = 1 Then                         'EXIT
        For counter = Check1.LBound To Check1.UBound
            Select Case counter
                Case 0, 4, 11, 12, 13, 17, 19, 23, 25, 30
                    Check1(counter).Enabled = False
            End Select
        Next counter
    End If
    
    If Check1(21).Value = 1 Then                        'PLAYER
        For counter = Check1.LBound To Check1.UBound
            Select Case counter
                Case 0, 3, 4, 11, 13, 18, 27
                    Check1(counter).Enabled = False
            End Select
        Next counter
    End If
    
    If Check1(24).Value = 1 Then                        'ROOM
        For counter = Check1.LBound To Check1.UBound
            Select Case counter
                Case 4, 11, 12, 13, 17, 19, 23, 25, 30
                    Check1(counter).Enabled = False
            End Select
        Next counter
    End If
                                                        'OBJECT
    If Check1(6).Value = 0 And Check1(21).Value = 0 And Check1(24).Value = 0 Then
        Check1(0).Enabled = False
        Check1(12).Enabled = False
    End If
    
    For counter = Check1.LBound To Check1.UBound
        Select Case counter
                Case 2, 6, 7, 21, 32, 29, 26, 24
                Case Else
                    If Check1(counter).Enabled = False Then Check1(counter).Value = 0
                End Select
    Next counter
End Sub

