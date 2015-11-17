import java.security.MessageDigest;
import java.io.*;
import javax.crypto.Cipher;
import javax.crypto.spec.*;
import java.util.*;
import java.math.BigInteger;

public class Jewel {
	public static String GetDeviceId() {
		String ok = "356280a58d3c437a45268a0b226d8cccad7b5dd28f5d1b37abf1873cc426a8a5";
		try {
			MessageDigest md = MessageDigest.getInstance("SHA-256");
			for (int i = 0; i < 10000000; i++) {
				if (i % 100000 == 0) {
					System.out.println("i=" + i);
				}
				String s = String.format("99999991%07d", i);
				md.update(s.getBytes());
				String h = (new BigInteger(md.digest())).toString(16);
				if (h.equals(ok)) {
					System.out.println("s=" + s);
					return s;
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
		return null;
	}
	public static void decode(String outFile, String inFile, String key) {
		try {
			Cipher c = Cipher.getInstance("AES/CBC/PKCS5Padding");
			IvParameterSpec iv = new IvParameterSpec("kLwC29iMc4nRMuE5".getBytes());
			File fs = new File(inFile);
			int size = (int)fs.length();
			System.out.println("size=" + size);
			FileInputStream ifs = new FileInputStream(fs);
			byte[] enc = new byte[size];
			ifs.read(enc);
			ifs.close();

			String s = "!" + key;
			SecretKeySpec sk = new SecretKeySpec(s.getBytes(), "AES");
			c.init(Cipher.DECRYPT_MODE, sk, iv);
			byte[] dec = c.doFinal(enc);

			FileOutputStream ofs = new FileOutputStream(outFile);
			ofs.write(dec);
			ofs.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	public static void main(String[] args) {
//		String id = GetDeviceId();
		String id = "999999913371337";
		decode("jewel.png", "jewel_c.png", id);
	}
}
