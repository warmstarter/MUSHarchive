Attribute VB_Name = "modGlobals"
Option Explicit

Public flashmode As Boolean

Public Function LeftJustify(myString As String, width As Integer) As String

    Dim rightPadding As String
    Dim i As Integer

    For i = 1 To width - Len(myString)
        rightPadding = rightPadding + Chr(32)
    Next

   LeftJustify = Left(myString, width) + rightPadding

End Function

