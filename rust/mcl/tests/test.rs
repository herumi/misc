use mcl::*;

macro_rules! field_test {
    ($t:ty) => {{
        let mut x = <$t>::zero();
        assert!(x.is_zero());
        assert!(!x.is_one());
        x.set_int(1);
        assert!(!x.is_zero());
        assert!(x.is_one());
        let mut y = <$t>::from_int(1);
        assert_eq!(x, y);
        y.set_int(2);
        assert!(x != y);
        x.set_str("65535", 10);
        y.set_str("ffff", 16);
        assert_eq!(x, y);
        x.set_int(123);
        assert!(x.is_odd());
        x.set_int(124);
        assert!(!x.is_odd());
        assert!(!x.is_negative());
        x.set_int(-124);
        assert!(x.is_negative());

        let mut z = unsafe { <$t>::uninit() };
        let mut w = unsafe { <$t>::uninit() };

        let a = 256;
        let b = 8;
        x.set_int(a);
        y.set_int(b);
        <$t>::add(&mut z, &x, &y);
        w.set_int(a + b);
        assert_eq!(z, w);

        <$t>::sub(&mut z, &x, &y);
        w.set_int(a - b);
        assert_eq!(z, w);

        <$t>::mul(&mut z, &x, &y);
        w.set_int(a * b);
        assert_eq!(z, w);

        <$t>::div(&mut z, &x, &y);
        w.set_int(a / b);
        assert_eq!(z, w);

        assert!(x.set_little_endian_mod(&[1, 2, 3, 4, 5]));
        assert_eq!(x.get_str(16), "504030201");
        <$t>::sqr(&mut y, &x);
        <$t>::mul(&mut z, &x, &x);
        assert_eq!(y, z);

        assert!(<$t>::square_root(&mut w, &y));
        if w != x {
            <$t>::neg(&mut z, &w);
            assert_eq!(x, z);
        }
    }};
}

#[test]
fn test() {
    assert!(init(CurveType::BLS12_381));
    assert_eq!(get_fp_serialized_size(), 48);
    assert_eq!(get_g1_serialized_size(), 48);
    assert_eq!(get_g2_serialized_size(), 48 * 2);
    assert_eq!(get_gt_serialized_size(), 48 * 12);
    assert_eq!(get_fr_serialized_size(), 32);

    // Fp
    assert_eq!(get_field_order(), "4002409555221667393417789825735904156556882819939007885332058136124031650490837864442687629129015664037894272559787");
    // Fr
    assert_eq!(
        get_curve_order(),
        "52435875175126190479447740508185965837690552500527637822603658699938581184513"
    );

    field_test!(Fr);
    field_test!(Fp);
}
