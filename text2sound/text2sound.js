/*
  how to use : cscript test2sound.js text-file
*/
var isWScript = true
if (WScript.FullName.indexOf('cscript.exe') >= 0) {
  isWScript = false
}

var args = WScript.Arguments

if (args.length == 0) {
  WScript.Echo('ERR cscript text2sound.js text-file')
  WScript.Quit()
}
var fs = new ActiveXObject("Scripting.FileSystemObject")
var textFile = args.Item(0)
//WScript.Echo('open ' + textFile)
var ifs = fs.OpenTextFile(textFile)
var text = ifs.ReadAll().split(/\r\n/)
ifs.Close()

var v = WScript.CreateObject("SAPI.SpVoice")
// v.Rate = 0 // slow if < 0, 0:normal, fast if > 0

for (var i = 0; i < text.length; i++) {
  var line = text[i]
  if (line == '@pause') {
    v.Pause()
    WScript.Echo('pause : type return key')
    if (!isWScript) {
      WScript.StdIn.ReadLine()
    }
    v.Resume()
  } else {
    v.Speak(line)
  }
}
