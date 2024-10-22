const std = @import("std");
const bls = @cImport({
    @cDefine("BLS_ETH", "");
    // MCLBN_COMPILED_TIME_VAR should be determined by the definition of BLS_ETH,
    // but since this is not working as intended, we explicitly provide a magic number.
    @cDefine("MCLBN_COMPILED_TIME_VAR", "246");
    @cInclude("bls/bls384_256.h");
});

const SecretKey = struct {
    v_: bls.blsSecretKey,
    pub fn setByCSPRNG(self: *SecretKey) void {
        const ret = bls.mclBnFr_setByCSPRNG(&self.v_.v);
        if (ret != 0) @panic("mclBnFr_setByCSPRNG");
    }
    // Returns a zero-length slice if the function fails.
    pub fn serialize(self: *const SecretKey, buf: *[]u8) []u8 {
        const len: usize = @intCast(bls.blsSecretKeySerialize(buf.*.ptr, buf.*.len, &self.v_));
        return buf.*[0..len];
    }
    pub fn deserialize(self: *SecretKey, buf: []const u8) bool {
        const len: usize = @intCast(bls.blsSecretKeyDeserialize(&self.v_, buf.ptr, buf.len));
        std.debug.print("len={} buf.len={}\n", .{ len, buf.len });
        return len > 0 and len == buf.len;
    }
    // set (buf[] as littleEndian) % r
    pub fn setLittleEndianMod(self: *SecretKey, buf: []const u8) void {
        const r = bls.mclBnFr_setLittleEndianMod(&self.v_.v, buf.ptr, buf.len);
        if (r < 0) @panic("mclBnFr_setLittleEndianMod");
    }
    // set (buf[] as bigEndian) % r
    pub fn setBigEndianMod(self: *SecretKey, buf: []const u8) void {
        const r = bls.mclBnFr_setBigEndianMod(&self.v_.v, buf.ptr, buf.len);
        if (r < 0) @panic("mclBnFr_setBigEndianMod");
    }
    pub fn setStr(self: *SecretKey, s: []const u8, base: i32) bool {
        const r = bls.mclBnFr_setStr(&self.v_.v, s.ptr, s.len, base);
        return r == 0;
    }
    // Returns a zero-length slice if the function fails.
    pub fn getStr(self: *const SecretKey, s: *[]u8, base: i32) []u8 {
        const len: usize = @intCast(bls.mclBnFr_getStr(s.*.ptr, s.*.len, &self.v_.v, base));
        return s.*[0..len];
    }
    pub fn getPublicKey(self: *const SecretKey) PublicKey {
        var pk: PublicKey = undefined;
        bls.blsGetPublicKey(&pk.v_, &self.v_);
        return pk;
    }
    pub fn sign(self: *const SecretKey, msg: []const u8) Signature {
        var sig: Signature = undefined;
        bls.blsSign(&sig.v_, &self.v_, msg.ptr, msg.len);
        return sig;
    }
};

const PublicKey = struct {
    v_: bls.blsPublicKey,
    // Returns a zero-length slice if the function fails.
    pub fn serialize(self: *const PublicKey, buf: *[]u8) []u8 {
        const len: usize = @intCast(bls.blsPublicKeySerialize(buf.*.ptr, buf.*.len, &self.v_));
        return buf.*[0..len];
    }
    pub fn deserialize(self: *PublicKey, buf: []const u8) bool {
        const len: usize = @intCast(bls.blsPublicKeyDeserialize(&self.v_, buf.ptr, buf.len));
        std.debug.print("len={} buf.len={}\n", .{ len, buf.len });
        return len > 0 and len == buf.len;
    }
    pub fn verify(self: *const PublicKey, sig: *const Signature, msg: []const u8) bool {
        return bls.blsVerify(&sig.v_, &self.v_, msg.ptr, msg.len) == 1;
    }
};

const Signature = struct {
    v_: bls.blsSignature,
    // Returns a zero-length slice if the function fails.
    pub fn serialize(self: *const Signature, buf: *[]u8) []u8 {
        const len: usize = @intCast(bls.blsSignatureSerialize(buf.*.ptr, buf.*.len, &self.v_));
        return buf.*[0..len];
    }
    pub fn deserialize(self: *Signature, buf: []const u8) bool {
        const len: usize = @intCast(bls.blsSignatureDeserialize(&self.v_, buf.ptr, buf.len));
        std.debug.print("len={} buf.len={}\n", .{ len, buf.len });
        return len > 0 and len == buf.len;
    }
};

pub fn main() void {
    {
        const res = bls.blsInit(bls.MCL_BLS12_381, bls.MCLBN_COMPILED_TIME_VAR);
        std.debug.print("res={}\n", .{res});
    }
    std.debug.print("{}\n", .{bls.blsGetG1ByteSize()});
    std.debug.print("size Fr={}\n", .{bls.MCLBN_FR_UNIT_SIZE});
    std.debug.print("size Fp={}\n", .{bls.MCLBN_FP_UNIT_SIZE});
    std.debug.print("size SecretKey={}\n", .{@sizeOf(bls.blsSecretKey)});
    var sec: SecretKey = undefined;
    var sec2: SecretKey = undefined;
    sec.setByCSPRNG();
    var buf128: [128]u8 = undefined;
    var buf: []u8 = &buf128;

    const cbuf: []u8 = sec.serialize(&buf);
    std.debug.print("sec:serialize={}\n", .{std.fmt.fmtSliceHexLower(cbuf)});
    if (sec2.deserialize(cbuf)) {
        std.debug.print("sec2:serialize={}\n", .{std.fmt.fmtSliceHexLower(sec2.serialize(&buf))});
    } else {
        std.debug.print("ERR sec2:serialize\n", .{});
    }
    std.debug.print("sec:getStr(10)={s}\n", .{sec.getStr(&buf, 10)});
    std.debug.print("sec:getStr(16)=0x{s}\n", .{sec.getStr(&buf, 16)});
    sec.setLittleEndianMod(@as([]const u8, &.{ 1, 2, 3, 4, 5 }));
    std.debug.print("sec={s}\n", .{sec.getStr(&buf, 16)});
    sec.setBigEndianMod(@as([]const u8, &.{ 1, 2, 3, 4, 5 }));
    std.debug.print("sec={s}\n", .{sec.getStr(&buf, 16)});
    if (sec.setStr("1234567890123", 10)) {
        std.debug.print("sec={s}\n", .{sec.getStr(&buf, 10)});
    }
    const pk = sec.getPublicKey();
    std.debug.print("pk={}\n", .{std.fmt.fmtSliceHexLower(pk.serialize(&buf))});
    const msg = "abcdefg";
    const sig = sec.sign(msg);
    std.debug.print("verify={}\n", .{pk.verify(&sig, msg)});
    std.debug.print("verify={}\n", .{pk.verify(&sig, "abc")});
}
