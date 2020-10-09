use std::thread;

fn add(a:i32, b:i32) -> i32 {
	println!("add a={} b={}\n", a, b);
	return a + b;
}

fn main() {
	let n = 5;
	let mut handles = vec![];
	for i in 0..n {
		let handle = thread::spawn(move|| {
			add(3, i);
			return (i, i+1)
		});
		handles.push(handle);
	}
	let mut sum = 0;
	let mut first = true;
	for h in handles {
		let v = h.join().unwrap();
		if first {
			sum = v.0;
			first = false;
		} else {
			sum = sum + v.0;
		}
		println!("v={:?}", v);
	}
	println!("sum {:?}\n", sum);
}

