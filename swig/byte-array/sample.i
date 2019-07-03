%module Sample

%include <std_string.i>

%typemap(jtype) void getStr "byte[]"
%typemap(jstype) void getStr "byte[]"
%typemap(jni) void getStr "jbyteArray"
%typemap(javaout) void getStr { return $jnicall; }
%typemap(in, numinputs=0) std::string& out (std::string buf) "$1=&buf;"
%typemap(argout) std::string& out {
  $result = JCALL1(NewByteArray, jenv, $1->size());
  JCALL4(SetByteArrayRegion, jenv, $result, 0, $1->size(), (const jbyte*)$1->c_str());
}


%inline %{

void getStr(std::string& out) throw(std::exception) {
    out = "abcd";
}

%}
