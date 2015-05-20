import java.io.*;
import mcl.test.*;

public class TestTest {
	static {
		System.loadLibrary("test_wrap");
	}
	public static void main(String argv[]) {
		try {
			System.out.println(":::A(5)");
			A a0 = new A(5);
			System.out.println(":::A(a0)");
			A a1 = new A(a0);
			System.out.println(":::addModify");
			a1.put();
			a1.addModify(5);
			a1.put();
			System.out.println(":::addReturn");
			A a3 = a1.addReturn(4);
			a1.put();
			a3.put();
			System.out.println(":::addModify");
			a3.addModify(2);
			a1.put();
			a3.put();
			System.out.println(":::assign");
			a3 = a1;
			a1.put();
			a3.put();
			System.out.println(":::AddRetrun with new");
			A a4 = new A(a0.addReturn(4));
			a1.put();
			a3.put();
		} catch (RuntimeException e) {
			System.out.println("unknown exception :" + e);
		}
	}
}
