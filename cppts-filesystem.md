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

主なクラスや構造体
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
主な関数
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
