%module Sample

%include <std_string.i>

%typemap(jtype) size_t getStr "byte[]"
%typemap(jstype) size_t getStr "byte[]"
%typemap(jni) size_t getStr "jbyteArray"
%typemap(javaout) size_t getStr { return $jnicall; }
%typemap(in, numinputs=0) std::string& out (std::string temp) "$1=&temp;"
%typemap(argout) std::string& out {
  $result = JCALL1(NewByteArray, jenv, $1->size());
  JCALL4(SetByteArrayRegion, jenv, $result, 0, $1->size(), (const jbyte*)$1->c_str());
}
// Optional: return NULL if the function returned false
%typemap(out) size_t getStr {
  if ($1 == 0) {
    return NULL;
  }
}

%inline %{

size_t getStr(std::string& out) {
    out = "abc";
    return 1;
}

%}
