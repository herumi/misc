import java.io.*;
import java.lang.ProcessBuilder;
import java.io.ByteArrayInputStream;

public class RunCmd {
	public static void main(String argv[]) {
		try {
			ProcessBuilder pb = null;
//			ProcessBuilder pb = new ProcessBuilder("./t.exe", "123");
			String a0 = argv.length > 0 ? argv[0] : "";
			String a1 = argv.length > 1 ? argv[1] : "";
			String a2 = argv.length > 2 ? argv[2] : "";
			String a3 = argv.length > 3 ? argv[3] : "";
			String a4 = argv.length > 4 ? argv[4] : "";
			String a5 = argv.length > 5 ? argv[5] : "";
			String a6 = argv.length > 6 ? argv[6] : "";
			System.out.println("a0 = " + a0 + " a1 = " + a1 + " a2 = " + a2 + " a3 = " + a3 + " a4 = " + a4 + " a5 = " + a5 + " a6 = " + a6);
			switch (argv.length) {
			case 1: pb = new ProcessBuilder(argv[0]); break;
			case 2: pb = new ProcessBuilder(argv[0], argv[1]); break;
			case 3: pb = new ProcessBuilder(argv[0], argv[1], argv[2]); break;
			case 4: pb = new ProcessBuilder(argv[0], argv[1], argv[2], argv[3]); break;
			case 5: pb = new ProcessBuilder(argv[0], argv[1], argv[2], argv[3], argv[4]); break;
			case 6: pb = new ProcessBuilder(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]); break;
			case 7: pb = new ProcessBuilder(argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]); break;
			default:
				System.out.println("no cmd");
				return;
			}
			pb.redirectErrorStream(true); // stderr -> stdout
			Process p = pb.start();
			BufferedInputStream bis = new BufferedInputStream(p.getInputStream());
			BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream("runcmd.log"));
			int ret = p.waitFor();
			System.out.println("ret=" + ret);
			int bufSize = 4096;
			byte[] buf = new byte[bufSize];
			int n = bis.read(buf, 0, bufSize);
			bos.write(buf, 0, n);
/*
			int bufSize = 4096;
			byte[] buf = new byte[bufSize];
			for (;;) {
				int n = bis.read(buf, 0, bufSize);
				if (n < 0) break;
				bos.write(buf, 0, n);
			}

			int ret = p.waitFor();
			System.out.println("ret=" + ret);
*/
			bis.close();
			bos.close();
		} catch (Exception e) {
			System.out.println("unknown exception :" + e);
		}
	}
};
