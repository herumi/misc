// BBS署名デモ - メインJavaScriptファイル

// グローバル変数
let bbs = null;
let secretKey = null;
let publicKey = null;
let signature = null;
let proof = null;
let messages = [];
let disclosedIndices = [];
let disclosedMessages = [];
let originalMessages = []; // 元のメッセージを保存
let originalDisclosedMessages = []; // 元の開示メッセージを保存
let proofNonce = null; // 証明生成時のnonceを保存

// 初期化
async function initBBS() {
    try {
        bbs = window.bbs;
        console.log('BBSライブラリを初期化中...');
        await bbs.init();
        console.log('BBSライブラリの初期化が完了しました');
    } catch (error) {
        console.error('BBSライブラリの初期化に失敗しました:', error);
        alert('BBSライブラリの初期化に失敗しました。ページを再読み込みしてください。');
    }
}

// 文字列をUint8Arrayに変換
function stringToUint8Array(str) {
    const encoder = new TextEncoder();
    return encoder.encode(str);
}

// Uint8Arrayを文字列に変換
function uint8ArrayToString(arr) {
    const decoder = new TextDecoder('utf-8');
    return decoder.decode(arr);
}

// 生成時刻の文字列をnonceとして生成（YYYYMMDDHHMMSS.mmmm形式）
function generateTimestampNonce() {
    const now = new Date();
    const year = now.getFullYear();
    const month = String(now.getMonth() + 1).padStart(2, '0');
    const day = String(now.getDate()).padStart(2, '0');
    const hours = String(now.getHours()).padStart(2, '0');
    const minutes = String(now.getMinutes()).padStart(2, '0');
    const seconds = String(now.getSeconds()).padStart(2, '0');
    const milliseconds = String(now.getMilliseconds()).padStart(4, '0');
    
    const timestamp = `${year}${month}${day}${hours}${minutes}${seconds}.${milliseconds}`;
    console.log('生成されたnonce（タイムスタンプ）:', timestamp);
    return stringToUint8Array(timestamp);
}

// データの最初32バイトを表示用に変換
function getPreview(data) {
    if (typeof data === 'string') {
        return data.substring(0, 64) + '...';
    }
    return 'データ形式エラー';
}

// タブ切り替え
function showTab(tabName) {
    // すべてのタブコンテンツを非表示
    const tabContents = document.querySelectorAll('.tab-content');
    tabContents.forEach(content => content.classList.remove('active'));

    // すべてのタブボタンを非アクティブ
    const tabs = document.querySelectorAll('.tab');
    tabs.forEach(tab => tab.classList.remove('active'));

    // 指定されたタブをアクティブ
    document.getElementById(tabName).classList.add('active');
    event.target.classList.add('active');
}

// 鍵生成
async function generateKeys() {
    const btn = document.getElementById('generateKeys');
    const loading = document.getElementById('keygenLoading');
    const result = document.getElementById('keygenResult');

    try {
        btn.disabled = true;
        loading.style.display = 'inline-block';

        // 秘密鍵を生成
        secretKey = new bbs.SecretKey();
        secretKey.init();

        // 公開鍵を取得
        publicKey = secretKey.getPublicKey();

        // 結果を表示
        const secretKeyHex = secretKey.serializeToHexStr();
        const publicKeyHex = publicKey.serializeToHexStr();

        document.getElementById('secretKeyPreview').textContent = getPreview(secretKeyHex);
        document.getElementById('publicKeyPreview').textContent = getPreview(publicKeyHex);

        result.style.display = 'block';
        console.log('鍵生成が完了しました');

    } catch (error) {
        console.error('鍵生成に失敗しました:', error);
        alert('鍵生成に失敗しました: ' + error.message);
    } finally {
        btn.disabled = false;
        loading.style.display = 'none';
    }
}

// 署名生成
async function generateSignature(event) {
    event.preventDefault();

    const btn = document.getElementById('signBtn');
    const loading = document.getElementById('signLoading');
    const result = document.getElementById('signResult');

    try {
        btn.disabled = true;
        loading.style.display = 'inline-block';

        // フォームから個人情報を取得
        const formData = {
            lastName: document.getElementById('lastName').value,
            firstName: document.getElementById('firstName').value,
            gender: document.getElementById('gender').value,
            prefecture: document.getElementById('prefecture').value,
            city: document.getElementById('city').value,
            address: document.getElementById('address').value,
            birthYear: document.getElementById('birthYear').value,
            birthMonth: document.getElementById('birthMonth').value,
            birthDay: document.getElementById('birthDay').value
        };

        // メッセージ配列を作成
        messages = [
            stringToUint8Array(formData.lastName),
            stringToUint8Array(formData.firstName),
            stringToUint8Array(formData.gender),
            stringToUint8Array(formData.prefecture),
            stringToUint8Array(formData.city),
            stringToUint8Array(formData.address),
            stringToUint8Array(formData.birthYear),
            stringToUint8Array(formData.birthMonth),
            stringToUint8Array(formData.birthDay)
        ];

        // 元のメッセージを保存
        originalMessages = messages.map(msg => new Uint8Array(msg));

        // 署名を生成
        signature = bbs.sign(secretKey, publicKey, messages);

        // 結果を表示
        const signatureHex = signature.serializeToHexStr();
        document.getElementById('signaturePreview').textContent = getPreview(signatureHex);

        result.style.display = 'block';
        console.log('署名生成が完了しました');

        // 他のタブのボタンを有効化
        document.getElementById('verifyBtn').disabled = false;
        document.getElementById('generateProofBtn').disabled = false;

    } catch (error) {
        console.error('署名生成に失敗しました:', error);
        alert('署名生成に失敗しました: ' + error.message);
    } finally {
        btn.disabled = false;
        loading.style.display = 'none';
    }
}

// 署名検証
async function verifySignature() {
    const btn = document.getElementById('verifyBtn');
    const loading = document.getElementById('verifyLoading');
    const result = document.getElementById('verifyResult');

    try {
        btn.disabled = true;
        loading.style.display = 'inline-block';

        // 署名を検証
        const isValid = bbs.verify(signature, publicKey, messages);

        // 結果を表示
        result.className = isValid ? 'result success' : 'result error';
        result.innerHTML = `
            <h3>署名検証結果</h3>
            <div class="status ${isValid ? 'ok' : 'ng'}">
                ${isValid ? 'OK - 署名は有効です' : 'NG - 署名は無効です'}
            </div>
        `;

        result.style.display = 'block';
        console.log('署名検証が完了しました:', isValid);

    } catch (error) {
        console.error('署名検証に失敗しました:', error);
        alert('署名検証に失敗しました: ' + error.message);
    } finally {
        btn.disabled = false;
        loading.style.display = 'none';
    }
}

// 証明生成
async function generateProof() {
    const btn = document.getElementById('generateProofBtn');
    const loading = document.getElementById('proofLoading');
    const result = document.getElementById('proofResult');

    try {
        btn.disabled = true;
        loading.style.display = 'inline-block';

        // 開示する項目を取得
        disclosedIndices = [];
        disclosedMessages = [];
        const fieldNames = ['姓', '名', '性別', '都道府県', '群市町村', '住所', '誕生年', '誕生月', '誕生日'];

        for (let i = 0; i < fieldNames.length; i++) {
            const radio = document.querySelector(`input[name="disclose_${i}"]:checked`);
            if (radio && radio.value === 'disclose') {
                disclosedIndices.push(i);
                disclosedMessages.push(messages[i]);
            }
        }

        if (disclosedIndices.length === 0) {
            alert('少なくとも1つの項目を開示する必要があります。');
            return;
        }

        // 元の開示メッセージを保存
        originalDisclosedMessages = disclosedMessages.map(msg => new Uint8Array(msg));

        // nonceを生成（実際のアプリケーションでは適切なnonceを使用）
        proofNonce = generateTimestampNonce();

        // 証明を生成
        proof = bbs.createProof(publicKey, signature, messages, new Uint32Array(disclosedIndices), proofNonce);

        // 結果を表示
        const proofHex = proof.serializeToHexStr();
        document.getElementById('proofPreview').textContent = getPreview(proofHex);

        result.style.display = 'block';
        console.log('証明生成が完了しました');

        // 証明検証タブのボタンを有効化
        document.getElementById('verifyProofBtn').disabled = false;

    } catch (error) {
        console.error('証明生成に失敗しました:', error);
        alert('証明生成に失敗しました: ' + error.message);
    } finally {
        btn.disabled = false;
        loading.style.display = 'none';
    }
}

// 証明検証
async function verifyProof() {
    const btn = document.getElementById('verifyProofBtn');
    const loading = document.getElementById('proofVerifyLoading');
    const result = document.getElementById('proofVerifyResult');

    try {
        btn.disabled = true;
        loading.style.display = 'inline-block';

        // nonceが存在するかチェック
        if (!proofNonce) {
            throw new Error('証明が生成されていません。先に証明を生成してください。');
        }

        // nonce（証明生成時と同じもの）
        const nonce = proofNonce;

        // 証明を検証
        const isValid = bbs.verifyProof(publicKey, proof, disclosedMessages, new Uint32Array(disclosedIndices), nonce);

        // 結果を表示
        result.className = isValid ? 'result success' : 'result error';
        result.innerHTML = `
            <h3>証明検証結果</h3>
            <div class="status ${isValid ? 'ok' : 'ng'}">
                ${isValid ? 'OK - 証明は有効です' : 'NG - 証明は無効です'}
            </div>
        `;

        result.style.display = 'block';
        console.log('証明検証が完了しました:', isValid);

    } catch (error) {
        console.error('証明検証に失敗しました:', error);
        alert('証明検証に失敗しました: ' + error.message);
    } finally {
        btn.disabled = false;
        loading.style.display = 'none';
    }
}

// 署名検証タブの情報を更新
function updateVerifyInfo() {
    if (messages.length === 0) return;

    const fieldNames = ['姓', '名', '性別', '都道府県', '群市町村', '住所', '誕生年', '誕生月', '誕生日'];
    const verifyMessages = document.getElementById('verifyMessages');
    const verifyEditControls = document.getElementById('verifyEditControls');
    const verifyEditFields = document.getElementById('verifyEditFields');
    
    // メッセージ情報を表示
    let html = '';
    messages.forEach((msg, index) => {
        html += `<div><strong>${fieldNames[index]}:</strong> ${uint8ArrayToString(msg)}</div>`;
    });
    verifyMessages.innerHTML = html;

    // 編集フィールドを生成
    html = '';
    messages.forEach((msg, index) => {
        const value = uint8ArrayToString(msg);
        html += `
            <div class="edit-field">
                <label for="verify_edit_${index}">${fieldNames[index]}</label>
                <input type="text" id="verify_edit_${index}" value="${value}" 
                       onchange="updateVerifyMessage(${index}, this.value)">
            </div>
        `;
    });
    verifyEditFields.innerHTML = html;
    verifyEditControls.style.display = 'block';
}

// 証明生成タブの情報を更新
function updateProofInfo() {
    if (messages.length === 0) return;

    const fieldNames = ['姓', '名', '性別', '都道府県', '群市町村', '住所', '誕生年', '誕生月', '誕生日'];
    const proofMessages = document.getElementById('proofMessages');
    const disclosureControls = document.getElementById('disclosureControls');
    
    // メッセージ情報を表示
    let html = '';
    messages.forEach((msg, index) => {
        html += `<div><strong>${fieldNames[index]}:</strong> ${uint8ArrayToString(msg)}</div>`;
    });
    proofMessages.innerHTML = html;

    // 開示制御を生成
    html = '';
    messages.forEach((msg, index) => {
        const value = uint8ArrayToString(msg);
        html += `
            <div class="disclosure-item">
                <label>
                    <input type="radio" name="disclose_${index}" value="disclose" checked>
                    開示する
                </label>
                <label>
                    <input type="radio" name="disclose_${index}" value="hide">
                    開示しない
                </label>
                <div class="field-value">${value}</div>
            </div>
        `;
    });
    disclosureControls.innerHTML = html;
    disclosureControls.style.display = 'grid';

    // ラジオボタンの変更イベントを追加
    messages.forEach((msg, index) => {
        const radios = document.querySelectorAll(`input[name="disclose_${index}"]`);
        const fieldValue = disclosureControls.children[index].querySelector('.field-value');
        
        radios.forEach(radio => {
            radio.addEventListener('change', function() {
                if (this.value === 'hide') {
                    fieldValue.textContent = '***';
                    fieldValue.classList.add('hidden');
                } else {
                    fieldValue.textContent = uint8ArrayToString(msg);
                    fieldValue.classList.remove('hidden');
                }
            });
        });
    });
}

// 証明検証タブの情報を更新
function updateProofVerifyInfo() {
    if (disclosedMessages.length === 0) return;

    const fieldNames = ['姓', '名', '性別', '都道府県', '群市町村', '住所', '誕生年', '誕生月', '誕生日'];
    const proofVerifyMessages = document.getElementById('proofVerifyMessages');
    const proofVerifyEditControls = document.getElementById('proofVerifyEditControls');
    const proofVerifyEditFields = document.getElementById('proofVerifyEditFields');
    
    // 開示メッセージ情報を表示
    let html = '';
    disclosedIndices.forEach((index, i) => {
        html += `<div><strong>${fieldNames[index]}:</strong> ${uint8ArrayToString(disclosedMessages[i])}</div>`;
    });
    proofVerifyMessages.innerHTML = html;

    // 編集フィールドを生成
    html = '';
    disclosedIndices.forEach((index, i) => {
        const value = uint8ArrayToString(disclosedMessages[i]);
        html += `
            <div class="edit-field">
                <label for="proof_verify_edit_${i}">${fieldNames[index]}</label>
                <input type="text" id="proof_verify_edit_${i}" value="${value}" 
                       onchange="updateProofVerifyMessage(${i}, this.value)">
            </div>
        `;
    });
    proofVerifyEditFields.innerHTML = html;
    proofVerifyEditControls.style.display = 'block';
}

// 署名検証用メッセージを更新
function updateVerifyMessage(index, value) {
    if (index >= 0 && index < messages.length) {
        messages[index] = stringToUint8Array(value);
        updateVerifyInfo(); // 表示を更新
    }
}

// 証明検証用メッセージを更新
function updateProofVerifyMessage(index, value) {
    if (index >= 0 && index < disclosedMessages.length) {
        disclosedMessages[index] = stringToUint8Array(value);
        updateProofVerifyInfo(); // 表示を更新
    }
}

// 署名検証メッセージをリセット
function resetVerifyMessages() {
    if (originalMessages.length > 0) {
        messages = originalMessages.map(msg => new Uint8Array(msg));
        updateVerifyInfo();
    }
}

// 証明検証メッセージをリセット
function resetProofVerifyMessages() {
    if (originalDisclosedMessages.length > 0) {
        disclosedMessages = originalDisclosedMessages.map(msg => new Uint8Array(msg));
        updateProofVerifyInfo();
    }
}

// イベントリスナーの設定
document.addEventListener('DOMContentLoaded', async function() {
    console.log('BBS署名デモを初期化中...');
    
    // BBSライブラリを初期化
    await initBBS();
    
    // フォームイベントを設定
    document.getElementById('signForm').addEventListener('submit', generateSignature);
    
    // タブ切り替え時に情報を更新
    const tabs = document.querySelectorAll('.tab');
    tabs.forEach(tab => {
        tab.addEventListener('click', function() {
            const tabName = this.getAttribute('onclick').match(/'([^']+)'/)[1];
            
            // タブに応じて情報を更新
            if (tabName === 'verify' && messages.length > 0) {
                updateVerifyInfo();
            } else if (tabName === 'proof' && messages.length > 0) {
                updateProofInfo();
            } else if (tabName === 'proof-verify' && disclosedMessages.length > 0) {
                updateProofVerifyInfo();
            }
        });
    });
    
    console.log('BBS署名デモの初期化が完了しました');
}); 