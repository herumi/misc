import java.io.*;
import sample.*;

public class SampleTest { 
	static {
		String lib = "sample";
		String libName = System.mapLibraryName(lib);
		System.out.println("libName : " + libName);
		System.loadLibrary(lib);
	}
	static String toHex(byte[] buf) {
		StringBuilder sb = new StringBuilder();
		for (byte b : buf) {
			sb.append(String.format("%02x", b));
		}
		return sb.toString();
	}
	public static void main(String[] argv) {
		byte[] buf = Sample.getStr();
		String hex = toHex(buf);
		System.out.println(hex);
	}
}
