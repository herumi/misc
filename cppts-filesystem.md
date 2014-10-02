# ファイルシステムライブラリ

## 参照

* [filesystem-ts](https://github.com/cplusplus/filesystem-ts)
* [N3505:Filesystem Library Proposal(Revision 4)](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3505.html)
* [Rapperswil会議での指摘](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4080.html)

## 目的

C++標準ライブラリにファイス操作関係のインタフェースを提供する。

## 概要

* C++でスクリプトのような安全で移植性の高いファイル操作を行えるようにする。
    * 現バージョンのC++はディレクトリ操作などを提供していない。
* Boostライブラリをベースにする。ある程度は[VC](http://msdn.microsoft.com/en-us/library/hh874694.aspx)に実装されている。

## Boost Filesystemのとの違い

* Boostはファイル名などにchar型の文字列を使う。他のエンコーディングはcodecvtを使う。
* このライブラリではlocaleを使う。またUTF-8をサポートする。こちらの方が便利で実用的だろう。

## パスのデザイン

* 従来のnarrow characterを扱うIO関係のC/C++ライブラリを注意深く取り込む必要がある。
    * POSIXならchar型, Windowsはchar型をwide型に変換している。
* 今までのに満足してる人はそのまま使いたいはず。
* 移植性の問題に困ってる人はchar16_tやchar32_tやUTF-8を使いたいだろう。

## 用語

* 絶対パス : それ以外の情報なしにファイルの位置を特定できる情報
* 正規パス : シンボリックリンクや.や..を含まない絶対パス
* ディレクトリ : ファイルや他のディレクトリエントリ情報を含むかのように振るまうファイル
* ファイル : ユーザやシステムデータを保持するファイルシステムの中にある対象物
読んだり書いたりできる。さまざまな属性を持つ。
* ファイルシステム : ファイルとそれらの属性の集合
* 複数のスレッドやプロセスが同時に同じ対象物を変更するとレースになる
* ファイル名 : ファイルの名前 .や..は特別な意味を持つ。次のものはOS依存
    * ファイル名に使える文字
    * 使えるファイル名
    * 特別なファイル名
    * 大文字小文字を区別するかしないか
    * ファイルの型、ディレクトリ、など特別なルール
* リンク : ファイル名に関連づけられたディレクトリエントリ
* nativeエンコーディング
* NTCTS(Null-Terminated Character-Type Sequenceの略)
* OS依存
* 親ディレクトリ : あるディレクトリに対して、それを含むディレクトリ ..で指し示す
.や..には適用されない
* パス : (ルート名)_opt (ルートディレクトリ)_opt ファイル名の列
* パス名分析?(pathname resolution) OS依存
see [POSIX : Pathname Resolution](http://pubs.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap04.html#tag_04_11)

# 要件
char, wchar_t, char16_t, char32_tをencoded文字型という(signed charやunsigned charは入らない)。
templateパラメータのcharTはこれらのうちのどれかである。

# ヘッダファイル<filesystem>

## 主なクラスや構造体
```
namespace std { namespace tbd { namespace filesystem {
    class path;
    class filesystem_error;
    class directory_entry;
    class directory_iterator;
    class recursiev_directory_iterator;
    class file_type;
    class perms;
    class file_status;
    struct space_info;
    enum class copy_options;
    enum class directory_options;
    file_time_type;
}}}
```
## 主な関数
殆どの関数はエラーコードerror_code ecを返すバージョンと例外を投げるバージョンの2種類が用意される。

* 例外バージョン : void copy(const path& from, const path& to);
* エラーを返すバージョン : void copy(const path& from, const path& to,error_code& ec) noexcept;

```
path absolute(const path& p, const path& base=current_path());
path canonical(const path& p, const path& base = current_path());
void copy(const path& from, const path& to);
bool copy_file(const path& from, const path& to);
void copy_symlink(const path& existing_symlink,const path& new_symlink);
bool create_directories(const path& p);
bool create_directory(const path& p);
void create_directory_symlink(const path& to,const path& new_symlink);
void create_hard_link(const path& to, const path& new_hard_link);
void create_symlink(const path& to, const path& new_symlink);
path current_path();
void current_path(const path& p);
bool exists(file_status s) noexcept;
bool exists(const path& p);
bool equivalent(const path& p1, const path& p2);
uintmax_t file_size(const path& p);
uintmax_t hard_link_count(const path& p);
bool is_block_file(file_status s) noexcept;
bool is_block_file(const path& p);
bool is_character_file(file_status s) noexcept;
bool is_character_file(const path& p);
bool is_directory(file_status s) noexcept;
bool is_directory(const path& p);
bool is_empty(const path& p);
bool is_fifo(file_status s) noexcept;
bool is_fifo(const path& p);
bool is_other(file_status s) noexcept;
bool is_other(const path& p);
bool is_regular_file(file_status s) noexcept;
bool is_regular_file(const path& p);
bool is_socket(file_status s) noexcept;
bool is_socket(const path& p);
bool is_symlink(file_status s) noexcept;
bool is_symlink(const path& p);
file_time_type last_write_time(const path& p);
void last_write_time(const path& p, file_time_type new_time);
path read_symlink(const path& p);
bool remove(const path& p);
uintmax_t remove_all(const path& p);
void rename(const path& from, const path& to);
void resize_file(const path& p, uintmax_t size);
space_info space(const path& p);
file_status status(const path& p);
bool status_known(file_status s) noexcept;
file_status symlink_status(const path& p);
path system_complete(const path& p);
path temp_directory_path();
path unique_path(const path& model="%%%%-%%%%-%%%%-%%%%");
```

# class path
class pathはパス名を表すクラス。
そのパスが実際に存在するかパス名がvalidである必要はない。
```
namespace std { namespace tbd { namespace filesystem {

class path {
public:
    typedef see below value_type;
    typedef basic_string<value_type> string_type;
    static constexpr value_type preferred_separator = see below;
    ...
private:
    string_type pathname; // exposition only
};
}}}
```
value_typeはOSが実際に使うencoded文字型。
POSIXならvalue_type=charでpreferred_separator=/。
Windowsならvalue_type=wchar_tでpreferred_separator=\。

## path名

```
pathname:
  root-name root-directory_opt relative-path_opt
  root-directory relative-path_opt
  relative-path
```

* root-name : 絶対パスを表すOS依存の識別子を指す名前。
たとえばc:とか\\とか。

```
root-directory:
  directory-separator

relative-path:
    filename
    relative-path directory-separator
    relative-path directory-separator filename

filename:
    name
    dot
    dot-dot

name:
    directory-seprator以外のOS依存の文字の列

dot : .
dot-dot : ..
slash : /

directory-separator:
    slash
    slash directory-separator
    preferred-separator
    preferred-separator directory-separator

preferred-separator:
    OS依存のディレクトリセパレータ。slashの同意語。
```
* 連続する複数のdirectory-separatorは一つのdirectory-separatorと見なす。
* .というファイル名はカレントディレクトリを示す。
* ..は親ディレクトリを示す。
* 特定のOSにとって特別な意味を持つ特別のファイル名があってよい。

## pathの変換
この節の変換はPOSIXともWindowsのシステムとも違うやりかたである。なぜなら
* generic formatはnative pathとして受け付ける。
* native formatとgeneric formatを区別する必要はない。
* 通常のファイルやディレクトリのためのパスは同じ文法である。

generic formatがnative pathとして受け入れられないときに限り変換が行われる。

OSによってはgeneric formatとnative formatの区別に曖昧なところがあるかもしれない。

## pathの型とエンコードの変換

pathが受け付ける型がpath::value_typeと異なっていれば変換される。

* 受け付ける型がcharのとき
    * POSIXならpath::value_typeはcharなのでcharからの変換は行われない。
    * Windowsならそのnative narrow encodingはWin APIにより決まる(多分日本語ならcp932として扱いUTF-16に変換される)。
* 受け付ける型がwchar_tのとき
    * 変換方法は未規定
    * Windowsならpath::value_typeはwchar_tなので変換は行われない。
* 受け付ける型がchar16_t, char32_tのとき
    * 変換方法は未規定

## pathが受け付ける文字列
受け付ける文字の型はいずれもencoding文字型のどれか。
* basic_string<charT>
* NTCSを指すイテレータ
* NTCSへのポインタ

```
template<class Source>
path(const Source& source, const locale& loc);

template<class InputIterator>
path(InputIterator begin, InputIterator end, const locale& loc);
```
SouceやInputIteratorの型はcharであること。
sourceや[begin, end)で示される値を用いてpathを生成する。

value_typeがwchar_tならlocのcodecvt<wchar_t, char> facetを使って変換する。

例 : ISO/IEC 8859-1に従ってエンコードされた文字列を読み込んでpathに設定する。
```
namespace fs = std::tbd::filesystem;
std::string latin1_string = read_latin1_data();
codecvt_8859_1<wchar_t> latin1_facet;
std::locale latin1_locale(std::locale(), latin1_facet);
fs::create_directory(fs::path(latin1_string, latin1_locale);
```
POSIXではlatin1_facetを使ってISO/IEC 8859-1にエンコードされたlatin1_stringを
native wideエンコーディングのwchat_t型に変換する。
その変換された文字列を現在のnative narrowエンコーディング文字列pathnameに変換する。
もしnative wideエンコーディングがUTF-16かUTF-32で、native narrowエンコーディングがUTF-8なら、
ISO/IEC 8859-1文字列は全てUniocde表現に変換できる。
しかし、他のnative narrowエンコーディングを使っていた場合は対応する表現がないかもしれない。

Windowsでは、latin1_facetを使ってISO/IEC 8859-1にエンコードされたlatin1_stringを
UTF-16エンコーディングであるwide文字列pathnameに変換する。
ISO/IEC 8859-1文字列はs全てUniocde表現に変換できる。

## pathの追加

```
path& operator/=(const path& p);
```
path::preferred_separatorをpathに追加してからp.native()をpathnameに追加する。
ただし、
* 追加するseparatorが冗長
* 追加すると相対パスが絶対パスになってしまう
* p.empty()
* *p.native().cbegin()がディレクトリのseparatorである
ときはpreferred_separatorを追加しない。

pathの連結なども同様。

## その他のパスの操作

* make_preferred(); preferred_separatorを使ったものに直される。
```
"foo/bar" => "foo/bar"
"foo\bar" =. "foo/bar"
```
* remove_filename(); ファイル名を取り除く。
```
"/foo" => "/"
"/" => ""
```
* replace_filename(); ファイル名を置き換える。
```
path("/foo").replace_filename("bar"); => "/bar"
path("/").replace_filename("bar"); => "bar"
```
* replace_extension(); 拡張子を取り替える。

## path nativeフォーマット
OSのnativeな文字列を返す。
```
const string_type& native() const noexcept;
```
pathnameを返す。
```
const value_type* c_str() const noexcept;
```
pathname.c_str()を返す。
```
operator string_type() const;
template <class charT, class traits = char_traits<charT>,
class Allocator = allocator<charT> >
basic_string<charT, traits, Allocator>
string(const Allocator& a = Allocator()) const;
string string() const;
wstring wstring() const;
string u8string() const;
u16string u16string() const;
u32string u32string() const;
```
それぞれのエンコーディングに従って変換してpathnameを返す。
u8string()はUTF-8である。

# path genericフォーマット
genericフォーマットを返す。ディレクトリseparatorは/である。
たとえば"foo\\bar"は"foot/bar"になる。

```
template <class charT, class traits = char_traits<charT>,
class Allocator = allocator<charT> >
basic_string<charT, traits, Allocator>
generic_string(const Allocator& a = Allocator()) const;
string generic_string() const;
wstring generic_wstring() const;
string generic_u8string() const;
u16string generic_u16string() const;
u32string generic_u32string() const;
```

## pathの分解

* root_name(); root-nameを返す。なければpath()。
* root_directory(); root-directoryを持ってればroot-directoryを返す。なければpath()。
* root_path(); = root_name() / root_directory()
* relative_path(); 空でなければroot-pathのあとのファイル名。
* parent_path(); 親ディレクトリ。
* filename(); ファイル名を返す。
```
"/foo/bar.txt" => "bar.txt"
"/" => "/"
"." => "."
".." => ".."
```
* stem(); filename()が.や..でなければ最後の.の前までの文字列。
```
"/foo/bar.txt" => "bar"
"foo.bar.baz.tar" => "foo.bar.baz"
"foo.bar.baz" => "foo.bar"
"foo.bar" => "foo"
```
* extension(); .つきの拡張子を返す。
```
"/foo/bar.txt" => ".txt"
```

# filesystem_error
```
class filesystem_error : public system_error {
public:
    filesystem_error(const string& what_arg, error_code ec);
    filesystem_error(const string& what_arg, const path& p1, error_code ec);
    filesystem_error(const string& what_arg,
    const path& p1, const path& p2, error_code ec);
    const path& path1() const noexcept;
    const path& path2() const noexcept;
    const char* what() const noexcept;
};
```
# file_type

名前     | 値 | 意味
---------|----|-----
none     |   0|ファイルの型を決定できなかった。または決定する間にエラーになった。
not_found|  -1|ファイルが無い。
regular  |   1|普通のファイル
directory|   2|ディレクトリファイル
symlink  |   3|シンボリックリンク
block    |   4|ブロック型
character|   5|キャラクタ型
fifo     |   6|FIFOかpipe
socket   |   7|ソケット
unknown  |   8|不明

# copy_option

ファイルが存在するときにどうするか。

名前              | 値 | 意味
------------------|----|-----
none              |   0|ファイルがあればエラー(デフォルト)。
skip_existing     |   1|ファイルがあれば上書きしない。エラーにしない。
overwirte_existing|   2|ファイルがあれば上書きする。
update_existing   |   4|ファイルが存在して古ければ置き換える。

サブディレクトリのファイルをどうするか。

名前      | 値 | 意味
----------|----|-----
none      |  0 |サブディレクトリに対して何もしない(デフォルト)。
recursive |  8 |再帰的にコピーする。

シンボリックリンクをどうするか。

名前         | 値 | 意味
-------------|----|-----
none         |   0|シンボリックリンクをたどる(デフォルト)。
copy_symlinks|  16|シンボリックリンクをシンボリックリンクとしてコピーする。
skip_symlinks|  32|シンボリックリンクを無視する。

コピーの方法

名前             | 値 | 意味
-----------------|----|-----
none             |   0|普通にコピーする(デフォルト)。
directories_only |  64|ディレクトリ構造のみをコピーする。ディレクトリ以外はコピーしない。
create_symlinks  | 128|ファイルをコピーする代わりにシンボリックリンクを作る。コピー先がカレントディレクトリでなければソースパスは絶対パスであるべき。
create_hard_links| 256|ファイルをコピーする代わりにハードリンクを作る。

パーミッション

名前            | 値    | POSIX |意味
----------------|-------|-------|---------------
none            |      0|       |何も許可しない。
owner_read      |   0400|S_IRUSR|所有者が読める。
owner_write     |   0200|S_IWUSR|所有者が書ける。
owner_exec      |   0100|S_IXUSR|所有者が実行できる。
owner_all       |   0700|S_IRWXU|所有者全部和。
group_read      |    040|S_IRGRP|グループで読める。
group_write     |    020|S_IWGRP|グループで書ける。
group_exec      |    010|S_IXGRP|グループで実行できる。
group_all       |    070|S_IRWXG|グループ全部和。
others_read     |     04|S_IROTH|他人が読める。
others_write    |     02|S_IWOTH|他人が書ける。
others_exec     |     01|S_IXOTH|他人が実行できる。
others_all      |     07|S_IRWXO|他人の全部和。
all             |   0777|       |owner_all|group_all|others_all
set_uid         |  04000|S_ISUID|set-user
set_gid         |  02000|S_ISGID|set-group
sticky_bit      |  01000|S_ISVTX|OS依存
mask            |  07777|       |all|set_uid|set_gid|sticky_bit
unknown         | 0xffff|       |パーミッション不明
add_perms       |0x10000|       |ビットごとにorするようにする。
remove_perms    |0x20000|       |設定したbitを0にする。
resolve_symlinks|0x40000|       |シンボリックリンク先に対して行う。
