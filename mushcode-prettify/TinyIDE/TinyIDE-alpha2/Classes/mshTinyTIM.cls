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

Private WithEvents mush As CSocket
Attribute mush.VB_VarHelpID = -1
Private myHOST As String 'local copy
Private myPORT As Long 'local copy
Private myError As String
Private myBufferSize As Long
Private buffer As String
Private myConnected As Boolean
Private userGags() As String
Private highlights() As String
Private m_reroute As Form
Attribute m_reroute.VB_VarHelpID = -1
Private m_ignore As String

Public Event DataArrived(ByVal buffer As String, color As Long)

Public Property Set reroute(newreroute As Form)
    Set m_reroute = newreroute
End Property

Public Property Get reroute() As Form
    Set reroute = m_reroute
End Property

Public Property Get ignore() As String
    ignore = m_ignore
End Property

Public Property Let ignore(ByVal ig As String)
    m_ignore = ig
End Property

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

Public Property Get SocketState() As String
    SocketState = mush.State
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
    
    mush.LocalPort = 0
    mush.RemoteHost = myHOST
    mush.RemotePort = myPORT
    mush.Connect
    
    Connect = True
    systemmessage ("Connected to TinyTIM!")
    myConnected = True
    
    Do Until mush.State = sckConnected
        DoEvents
    Loop
    
    If username <> "NO MATCH" And password <> "NO MATCH" Then
        SendData ("connect " & username & " " & password)
    End If
    Exit Function

errorCatch:
    Connect = False
    systemmessage ("Unable to connect to MUSH")
End Function



Private Sub Class_Initialize()
    myHOST = "yay.tim.org"
    myPORT = 5440
    myBufferSize = 4098
    Set mush = New CSocket
    ReDim userGags(0)
    ReDim highlights(0)
    loadgags userGags
    loadhighlights highlights
End Sub

Private Sub Class_Terminate()
    mush.CloseSocket
    recordgags userGags
    recordhighlights highlights
End Sub



Private Sub MUSH_onClose()
    systemmessage ("CONNECTION TO HOST HAS BEEN CLOSED")
    myConnected = False
End Sub



Private Sub MUSH_onDataArrival(ByVal bytesTotal As Long)
    Dim TEMP As String
    Dim lines() As String
    Dim counter As Long
    
    mush.GetData TEMP, vbString, bytesTotal
    
    If m_ignore <> "" Then
        If InStr(TEMP, m_ignore) = 0 Then
            Exit Sub
        Else
            m_ignore = ""
        End If
    End If
    
    If m_reroute Is Nothing Then
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
    Else
        m_reroute.rerouted TEMP
        Set m_reroute = Nothing
    End If
End Sub


Public Function SendData(data As String) As Boolean
    Dim TEMP() As Byte
    Dim pos As Long
       
    ReDim TEMP(Len(data) + 2)
      
    For pos = 1 To Len(data)
        TEMP(pos) = CByte(Asc(Mid(data, pos, 1)))
    Next
   
    mush.SendData TEMP
    mush.SendData 10
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
                        systemmessage ("Screen Flash Activated")
                    Case "OFF"
                        flashmode = False
                        systemmessage ("Screen Flash Disabled")
                    Case Else
                        systemmessage ("USAGE: /FLASH {ON | OFF}")
                End Select
            Else
                systemmessage ("USAGE: /FLASH {ON | OFF}")
            End If
            
        Case "GAG", "G"
            If UBound(args) Then
                userGags(UBound(userGags)) = args(1)
                ReDim Preserve userGags(UBound(userGags) + 1)
                systemmessage ("User " & args(1) & " has been gagged.")
            Else
                systemmessage ("USAGE: /{GAG | G} username")
            End If
        
        Case "HIGHLIGHT", "H"
            If UBound(args) Then
                highlights(UBound(highlights)) = args(1)
                ReDim Preserve highlights(UBound(highlights) + 1)
                systemmessage ("Any line containing '" & args(1) & "' will be highlighed.")
            Else
                systemmessage ("USAGE: /{HIGHLIGHT | H} value")
            End If
            
        Case "USERNAME"
            If UBound(args) Then
                
                If AddToINI("USER", "username", args(1), App.Path & "\mush.ini") Then
                    systemmessage "You auto-connect username has been set to " & args(1)
                Else
                    systemmessage "There has been an error while setting your username"
                End If
                
            Else
                systemmessage "USAGE: /USERNAME your_username"
            End If
            
        Case "PASSWORD"
            If UBound(args) Then
                
                If AddToINI("USER", "password", args(1), App.Path & "\mush.ini") Then
                    systemmessage "You auto-connect password has been set to " & args(1)
                Else
                    systemmessage "There has been an error while setting your password"
                End If
                
            Else
                systemmessage "USAGE: /PASSWORD your_password"
            End If
            
        Case "GAGS"
            systemmessage "GAGS----------------------------------"
            For counter = LBound(userGags) To UBound(userGags)
                If userGags(counter) <> "" Then
                    systemmessage "User " & userGags(counter) & " is gagged."
                End If
            Next counter
            systemmessage "END GAGS------------------------------"
            
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
                    systemmessage "Gag removed from " & args(1)
                Else
                    systemmessage "No such gag."
                End If
            Else
                systemmessage "USAGE: /{UNGAG | UGAG | UG} username"
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
                    systemmessage "Highlight removed from " & args(1)
                Else
                    systemmessage "No such highlight."
                End If
            Else
                systemmessage "USAGE: /{UNHIGHLIGHT | UHIGHLIGHT | UH} value"
            End If
        
        Case "HIGHLIGHTS"
            systemmessage "HIGHLIGHTS--------------------------"
            For counter = LBound(highlights) To UBound(highlights)
                If highlights(counter) <> "" Then
                    systemmessage "String " & highlights(counter) & " is highlighted."
                End If
            Next counter
            systemmessage "END HIGHLIGHTS----------------------"
            
        Case "FLAGS"
            If UBound(args) Then
                Dim flagform As frmFlags
                Set flagform = New frmFlags
                flagform.myobject = args(1)
                Set flagform.mush = Me
                flagform.Show , frmMain
            Else
                systemmessage "USAGE: /FLAGS {objectname | objectnumber}"
            End If
        
        Case "EDIT"
            If UBound(args) Then
                args = Split(command, " ")
                Dim editform As frmEdit
                Set editform = New frmEdit
                Set editform.mush = Me
                editform.myobject = args(1)
                editform.attr = args(2)
                editform.Show , frmMain
            Else
                systemmessage "USAGE: /EDIT {objectname | objectnumber} attribute"
            End If
        Case Else
            systemmessage ("UNRECOGNIZED COMMAND")

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

Public Sub systemmessage(message As String)
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

