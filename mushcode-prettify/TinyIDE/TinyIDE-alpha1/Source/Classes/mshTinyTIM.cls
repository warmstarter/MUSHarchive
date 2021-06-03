VERSION 1.0 CLASS
BEGIN
  MultiUse = -1  'True
  Persistable = 0  'NotPersistable
  DataBindingBehavior = 0  'vbNone
  DataSourceBehavior  = 0  'vbNone
  MTSTransactionMode  = 0  'NotAnMTSObject
END
Attribute VB_Name = "mshTinyTIM"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = True
Attribute VB_PredeclaredId = False
Attribute VB_Exposed = False
Attribute VB_Ext_KEY = "SavedWithClassBuilder6" ,"Yes"
Attribute VB_Ext_KEY = "Top_Level" ,"Yes"
Option Explicit

Private WithEvents MUSH As CSocket
Attribute MUSH.VB_VarHelpID = -1
Private myHOST As String 'local copy
Private myPORT As Long 'local copy
Private myError As String
Private myBufferSize As Long
Private buffer As String
Private myConnected As Boolean
Private userGags() As String
Private highlights() As String

Public Event DataArrived(ByVal buffer As String, color As Long)

Public Property Get connected() As Boolean
    connected = myConnected
End Property

Public Property Let Port(ByVal vData As Long)
    myPORT = vData
End Property


Public Property Get Port() As Long
    Port = myPORT
End Property


Public Property Let HOST(ByVal vData As String)
    myHOST = vData
End Property


Public Property Get HOST() As String
    HOST = myHOST
End Property


Public Function Connect() As Boolean
#If debugMode Then
#Else
    On Error GoTo errorCatch
#End If
    Dim username As String
    Dim password As String
    
    username = GetFromINI("USER", "username", "set_me", App.Path & "\mush.ini")
    password = GetFromINI("USER", "password", "default", App.Path & "\mush.ini")
    
    MUSH.LocalPort = 0
    MUSH.RemoteHost = myHOST
    MUSH.RemotePort = myPORT
    MUSH.Connect
    
    Connect = True
    SystemMessage ("Connected to TinyTIM!")
    myConnected = True
    
    Do Until MUSH.State = sckConnected
        DoEvents
    Loop
    
    If username <> "NO MATCH" And password <> "NO MATCH" Then
        SendData ("connect " & username & " " & password)
    End If
    Exit Function

errorCatch:
    Connect = False
    SystemMessage ("Unable to connect to MUSH")
End Function


Private Sub Class_Initialize()
    myHOST = "yay.tim.org"
    myPORT = 5440
    myBufferSize = 4098
    Set MUSH = New CSocket
    ReDim userGags(0)
    ReDim highlights(0)
    loadgags userGags
    loadhighlights highlights
End Sub

Private Sub Class_Terminate()
    MUSH.CloseSocket
    recordgags userGags
    recordhighlights highlights
End Sub

Private Sub MUSH_onClose()
    SystemMessage ("CONNECTION TO HOST HAS BEEN CLOSED")
    myConnected = False
End Sub



Private Sub MUSH_onDataArrival(ByVal bytesTotal As Long)
    Dim TEMP As String
    Dim lines() As String
    Dim counter As Long
    
    MUSH.GetData TEMP, vbString, bytesTotal
    lines = Split(TEMP, Chr(10))
    
    For counter = LBound(lines) To UBound(lines)
        If Not Gagged(lines(counter)) Then
            If Highlighted(lines(counter)) Then
                DisplayColor lines(counter), Highlighted(lines(counter))
            Else
                DisplayData lines(counter)
            End If
        End If
    Next counter
End Sub


Public Function SendData(data As String) As Boolean
    Dim TEMP() As Byte
    Dim pos As Long
       
    ReDim TEMP(Len(data) + 2)
      
    For pos = 1 To Len(data)
        TEMP(pos) = CByte(Asc(Mid(data, pos, 1)))
    Next
   
    MUSH.SendData TEMP
    MUSH.SendData 10
End Function

Public Function commandMode(command As String) As Boolean
    Dim args() As String
    Dim counter As Long
    command = Replace(command, "/", "")
    args = Split(command, " ", 2)
    
    Select Case UCase(args(0))
    
        Case "FLASH"
            If UBound(args) Then
                Select Case UCase(args(1))
                    Case "ON"
                        flashmode = True
                        SystemMessage ("Screen Flash Activated")
                    Case "OFF"
                        flashmode = False
                        SystemMessage ("Screen Flash Disabled")
                    Case Else
                        SystemMessage ("USAGE: /FLASH {ON | OFF}")
                End Select
            Else
                SystemMessage ("USAGE: /FLASH {ON | OFF}")
            End If
            
        Case "GAG", "G"
            If UBound(args) Then
                userGags(UBound(userGags)) = args(1)
                ReDim Preserve userGags(UBound(userGags) + 1)
                SystemMessage ("User " & args(1) & " has been gagged.")
            Else
                SystemMessage ("USAGE: /{GAG | G} username")
            End If
        
        Case "HIGHLIGHT", "H"
            If UBound(args) Then
                highlights(UBound(highlights)) = args(1)
                ReDim Preserve highlights(UBound(highlights) + 1)
                SystemMessage ("Any line containing '" & args(1) & "' will be highlighed.")
            Else
                SystemMessage ("USAGE: /{HIGHLIGHT | H} value")
            End If
            
        Case "USERNAME"
            If UBound(args) Then
                
                If AddToINI("USER", "username", args(1), App.Path & "\mush.ini") Then
                    SystemMessage "You auto-connect username has been set to " & args(1)
                Else
                    SystemMessage "There has been an error while setting your username"
                End If
                
            Else
                SystemMessage "USAGE: /USERNAME your_username"
            End If
            
        Case "PASSWORD"
            If UBound(args) Then
                
                If AddToINI("USER", "password", args(1), App.Path & "\mush.ini") Then
                    SystemMessage "You auto-connect password has been set to " & args(1)
                Else
                    SystemMessage "There has been an error while setting your password"
                End If
                
            Else
                SystemMessage "USAGE: /PASSWORD your_password"
            End If
            
        Case "GAGS"
            SystemMessage "GAGS----------------------------------"
            For counter = LBound(userGags) To UBound(userGags)
                If userGags(counter) <> "" Then
                    SystemMessage "User " & userGags(counter) & " is gagged."
                End If
            Next counter
            SystemMessage "END GAGS------------------------------"
            
        Case "UNGAG", "UGAG", "UG"
            Dim found As Boolean
            
            If UBound(args) Then
                For counter = LBound(userGags) To UBound(userGags)
                    If userGags(counter) = args(1) Then
                        found = True
                        Exit For
                    End If
                Next counter
        
                If found Then
                    userGags(counter) = ""
                    SystemMessage "Gag removed from " & args(1)
                Else
                    SystemMessage "No such gag."
                End If
            Else
                SystemMessage "USAGE: /{UNGAG | UGAG | UG} username"
            End If
            
        Case "UNHIGHLIGHT", "UHIGHLIGHT", "UH"
            If UBound(args) Then
                For counter = LBound(highlights) To UBound(highlights)
                    If highlights(counter) = args(1) Then
                        found = True
                        Exit For
                    End If
                Next counter
        
                If found Then
                    highlights(counter) = ""
                    SystemMessage "Highlight removed from " & args(1)
                Else
                    SystemMessage "No such highlight."
                End If
            Else
                SystemMessage "USAGE: /{UNHIGHLIGHT | UHIGHLIGHT | UH} value"
            End If
        
        Case "HIGHLIGHTS"
            SystemMessage "HIGHLIGHTS--------------------------"
            For counter = LBound(highlights) To UBound(highlights)
                If highlights(counter) <> "" Then
                    SystemMessage "String " & highlights(counter) & " is highlighted."
                End If
            Next counter
            SystemMessage "END HIGHLIGHTS----------------------"
        Case Else
            SystemMessage ("UNRECOGNIZED COMMAND")

    End Select
End Function

Private Function Gagged(myString As String) As Boolean
    Dim counter As Long
    
    For counter = LBound(userGags) To UBound(userGags)
        If InStr(1, myString, "You sense that " & userGags(counter), vbTextCompare) Then
            Gagged = True
            Exit Function
        End If
        
        If InStr(1, myString, "You sense that, somewhere, " & userGags(counter), vbTextCompare) Then
            Gagged = True
            Exit Function
        End If
        
        If UCase(Left(myString, Len(userGags(counter)))) = UCase(userGags(counter)) And userGags(counter) <> "" Then
            Gagged = True
            Exit Function
        End If
    Next counter
    
    Gagged = False
End Function

Private Function Highlighted(myString As String) As Long
    Dim counter As Long
    
    Highlighted = 0
    
    For counter = LBound(highlights) To UBound(highlights)
        If highlights(counter) <> "" Then
            If InStr(1, myString, highlights(counter), vbTextCompare) Then Highlighted = vbCyan
        End If
    Next counter
End Function

Private Sub SystemMessage(message As String)
    RaiseEvent DataArrived(message, vbYellow)
End Sub

Private Sub DisplayData(message As String)
    RaiseEvent DataArrived(message, vbWhite)
End Sub

Private Sub DisplayColor(message As String, color As Long)
    RaiseEvent DataArrived(message, color)
End Sub

Private Sub recordgags(gags() As String)
    Dim counter As Long
    Call AddToINI("GAGS", vbNullString, vbNullString, App.Path & "\mush.ini")
    
    For counter = LBound(gags) To UBound(gags)
        If gags(counter) <> "" Then
            Call AddToINI("GAGS", "gag" & counter, gags(counter), App.Path & "\mush.ini")
        End If
    Next counter
End Sub

Private Sub loadgags(gags() As String)
    Dim gag As String
    Dim counter As Long
    counter = 0
    Do While GetFromINI("GAGS", "gag" & counter, "NO MATCH", App.Path & "\mush.ini") <> "NO MATCH"
        gag = GetFromINI("GAGS", "gag" & counter, "NO MATCH", App.Path & "\mush.ini")
        gags(UBound(gags)) = gag
        ReDim Preserve gags(UBound(gags) + 1)
        counter = counter + 1
    Loop
End Sub

Private Sub recordhighlights(highlights() As String)
    Dim counter As Long
    Call AddToINI("HIGHLIGHTS", vbNullString, vbNullString, App.Path & "\mush.ini")
    
    For counter = LBound(highlights) To UBound(highlights)
        If highlights(counter) <> "" Then
            Call AddToINI("HIGHLIGHTS", "highlight" & counter, highlights(counter), App.Path & "\mush.ini")
        End If
    Next counter
End Sub

Private Sub loadhighlights(highlights() As String)
    Dim highlight As String
    Dim counter As Long
    counter = 0
    Do While GetFromINI("HIGHLIGHTS", "highlight" & counter, "NO MATCH", App.Path & "\mush.ini") <> "NO MATCH"
        highlight = GetFromINI("HIGHLIGHTS", "highlight" & counter, "NO MATCH", App.Path & "\mush.ini")
        highlights(UBound(highlights)) = highlight
        ReDim Preserve highlights(UBound(highlights) + 1)
        counter = counter + 1
    Loop
End Sub

