// @ts-nocheck
// BBS署名デモ - メインTypeScriptファイル

// 型定義（必要に応じて拡張）
declare global {
    interface Window {
        bbs: any;
    }
}

let bbs: any = null;
let secretKey: any = null;
let publicKey: any = null;
let signature: any = null;
let proof: any = null;
let messages: Uint8Array[] = [];
let disclosedIndices: number[] = [];
let disclosedMessages: Uint8Array[] = [];
let originalMessages: Uint8Array[] = [];
let originalDisclosedMessages: Uint8Array[] = [];
let proofNonce: Uint8Array | null = null;
let currentLanguage: 'ja' | 'en' = 'ja';

// 多言語対応テキスト
type Translations = Record<string, Record<string, string>>;
const translations: Translations = {
    ja: {
        // フィールド名
        lastName: '姓',
        firstName: '名',
        gender: '性別',
        prefecture: '都道府県',
        city: '群市町村',
        address: '住所',
        birthYear: '誕生年',
        birthMonth: '誕生月',
        birthDay: '誕生日',
        
        // 性別オプション
        male: '男',
        female: '女',
        other: 'その他',
        pleaseSelect: '選択してください',
        
        // 開示制御
        disclose: '開示する',
        hide: '開示しない',
        
        // メッセージ
        keyGenerationComplete: '鍵生成が完了しました',
        signatureGenerationComplete: '署名生成が完了しました',
        signatureVerificationComplete: '署名検証が完了しました',
        proofGenerationComplete: '証明生成が完了しました',
        proofVerificationComplete: '証明検証が完了しました',
        keyGenerationFailed: '鍵生成に失敗しました',
        signatureGenerationFailed: '署名生成に失敗しました',
        signatureVerificationFailed: '署名検証に失敗しました',
        proofGenerationFailed: '証明生成に失敗しました',
        proofVerificationFailed: '証明検証に失敗しました',
        bbsInitFailed: 'BBSライブラリの初期化に失敗しました。ページを再読み込みしてください。',
        atLeastOneItemRequired: '少なくとも1つの項目を開示する必要があります。',
        proofNotGenerated: '証明が生成されていません。先に証明を生成してください。',
        
        // 検証結果
        signatureValid: 'OK - 署名は有効です',
        signatureInvalid: 'NG - 署名は無効です',
        proofValid: 'OK - 証明は有効です',
        proofInvalid: 'NG - 証明は無効です',
        
        // タイトル
        signatureVerificationResult: '署名検証結果',
        proofVerificationResult: '証明検証結果'
    },
    en: {
        // フィールド名
        lastName: 'Last Name',
        firstName: 'First Name',
        gender: 'Gender',
        prefecture: 'Prefecture',
        city: 'City',
        address: 'Address',
        birthYear: 'Birth Year',
        birthMonth: 'Birth Month',
        birthDay: 'Birth Day',
        
        // 性別オプション
        male: 'Male',
        female: 'Female',
        other: 'Other',
        pleaseSelect: 'Please select',
        
        // 開示制御
        disclose: 'Disclose',
        hide: 'Hide',
        
        // メッセージ
        keyGenerationComplete: 'Key generation completed',
        signatureGenerationComplete: 'Signature generation completed',
        signatureVerificationComplete: 'Signature verification completed',
        proofGenerationComplete: 'Proof generation completed',
        proofVerificationComplete: 'Proof verification completed',
        keyGenerationFailed: 'Key generation failed',
        signatureGenerationFailed: 'Signature generation failed',
        signatureVerificationFailed: 'Signature verification failed',
        proofGenerationFailed: 'Proof generation failed',
        proofVerificationFailed: 'Proof verification failed',
        bbsInitFailed: 'BBS library initialization failed. Please reload the page.',
        atLeastOneItemRequired: 'At least one item must be disclosed.',
        proofNotGenerated: 'Proof has not been generated. Please generate a proof first.',
        
        // 検証結果
        signatureValid: 'OK - Signature is valid',
        signatureInvalid: 'NG - Signature is invalid',
        proofValid: 'OK - Proof is valid',
        proofInvalid: 'NG - Proof is invalid',
        
        // タイトル
        signatureVerificationResult: 'Signature Verification Result',
        proofVerificationResult: 'Proof Verification Result'
    }
};

// 言語切り替え機能
function switchLanguage(lang: 'ja' | 'en'): void {
    currentLanguage = lang;
    
    // 言語ボタンの状態を更新
    const langJa = document.getElementById('langJa');
    const langEn = document.getElementById('langEn');
    if (langJa) langJa.classList.toggle('active', lang === 'ja');
    if (langEn) langEn.classList.toggle('active', lang === 'en');
    
    // HTMLのlang属性を更新
    document.documentElement.lang = lang;
    
    // ページタイトルを更新
    const title = document.querySelector('title');
    if (title) {
        title.textContent = title.getAttribute(`data-${lang}`);
    }
    
    // すべてのdata属性を持つ要素のテキストを更新
    const elements = document.querySelectorAll('[data-ja][data-en]');
    elements.forEach(element => {
        const text = element.getAttribute(`data-${lang}`);
        if (text) {
            element.textContent = text;
        }
    });
    
    // 動的に生成されるコンテンツを更新
    updateDynamicContent();
}

// 動的コンテンツを更新
function updateDynamicContent(): void {
    // 署名検証タブの情報を更新
    if (messages.length > 0) {
        updateVerifyInfo();
    }
    
    // 証明生成タブの情報を更新
    if (messages.length > 0) {
        updateProofInfo();
    }
    
    // 証明検証タブの情報を更新
    if (disclosedMessages.length > 0) {
        updateProofVerifyInfo();
    }
}

// 翻訳テキストを取得
function t(key: string): string {
    return translations[currentLanguage][key] || key;
}

// 初期化
async function initBBS(): Promise<void> {
    try {
        bbs = window.bbs;
        console.log('BBSライブラリを初期化中...');
        await bbs.init();
        console.log('BBSライブラリの初期化が完了しました');
    } catch (error) {
        console.error('BBSライブラリの初期化に失敗しました:', error);
        alert(t('bbsInitFailed'));
    }
}

// 文字列をUint8Arrayに変換
function stringToUint8Array(str: string): Uint8Array {
    const encoder = new TextEncoder();
    return encoder.encode(str);
}

// Uint8Arrayを文字列に変換
function uint8ArrayToString(arr: Uint8Array): string {
    const decoder = new TextDecoder('utf-8');
    return decoder.decode(arr);
}

// 生成時刻の文字列をnonceとして生成（YYYYMMDDHHMMSS.mmmm形式）
function generateTimestampNonce(): Uint8Array {
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
function getPreview(data: string): string {
    if (typeof data === 'string') {
        return data.substring(0, 64) + '...';
    }
    return 'データ形式エラー';
}

// タブ切り替え
function showTab(tabName: string): void {
    // すべてのタブコンテンツを非表示
    const tabContents = document.querySelectorAll('.tab-content');
    tabContents.forEach(content => content.classList.remove('active'));

    // すべてのタブボタンを非アクティブ
    const tabs = document.querySelectorAll('.tab');
    tabs.forEach(tab => tab.classList.remove('active'));

    // 指定されたタブをアクティブ
    const targetTab = document.getElementById(tabName);
    if (targetTab) targetTab.classList.add('active');
    if (event && event.target) {
        (event.target as HTMLElement).classList.add('active');
    }

    // タブに応じて情報を更新
    if (tabName === 'verify' && messages.length > 0) {
        updateVerifyInfo();
    } else if (tabName === 'proof' && messages.length > 0) {
        updateProofInfo();
    } else if (tabName === 'proof-verify' && disclosedMessages.length > 0) {
        updateProofVerifyInfo();
    }
}

// 鍵生成
async function generateKeys(): Promise<void> {
    const btn = document.getElementById('generateKeys') as HTMLButtonElement;
    const loading = document.getElementById('keygenLoading') as HTMLElement;
    const result = document.getElementById('keygenResult') as HTMLElement;

    try {
        if (btn) btn.disabled = true;
        if (loading) loading.style.display = 'inline-block';

        // 秘密鍵を生成
        secretKey = new bbs.SecretKey();
        secretKey.init();

        // 公開鍵を取得
        publicKey = secretKey.getPublicKey();

        // 結果を表示
        const secretKeyHex = secretKey.serializeToHexStr();
        const publicKeyHex = publicKey.serializeToHexStr();

        const secretKeyPreview = document.getElementById('secretKeyPreview');
        const publicKeyPreview = document.getElementById('publicKeyPreview');
        if (secretKeyPreview) secretKeyPreview.textContent = getPreview(secretKeyHex);
        if (publicKeyPreview) publicKeyPreview.textContent = getPreview(publicKeyHex);

        if (result) result.style.display = 'block';
        console.log(t('keyGenerationComplete'));

    } catch (error) {
        console.error(t('keyGenerationFailed'), error);
        alert(t('keyGenerationFailed') + ': ' + (error as Error).message);
    } finally {
        if (btn) btn.disabled = false;
        if (loading) loading.style.display = 'none';
    }
}

// 署名生成
async function generateSignature(event: Event): Promise<void> {
    event.preventDefault();

    const btn = document.getElementById('signBtn') as HTMLButtonElement;
    const loading = document.getElementById('signLoading') as HTMLElement;
    const result = document.getElementById('signResult') as HTMLElement;

    try {
        if (btn) btn.disabled = true;
        if (loading) loading.style.display = 'inline-block';

        // フォームから個人情報を取得
        const formData = {
            lastName: (document.getElementById('lastName') as HTMLInputElement)?.value || '',
            firstName: (document.getElementById('firstName') as HTMLInputElement)?.value || '',
            gender: (document.getElementById('gender') as HTMLSelectElement)?.value || '',
            prefecture: (document.getElementById('prefecture') as HTMLInputElement)?.value || '',
            city: (document.getElementById('city') as HTMLInputElement)?.value || '',
            address: (document.getElementById('address') as HTMLInputElement)?.value || '',
            birthYear: (document.getElementById('birthYear') as HTMLInputElement)?.value || '',
            birthMonth: (document.getElementById('birthMonth') as HTMLInputElement)?.value || '',
            birthDay: (document.getElementById('birthDay') as HTMLInputElement)?.value || ''
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
        const signaturePreview = document.getElementById('signaturePreview');
        if (signaturePreview) signaturePreview.textContent = getPreview(signatureHex);

        if (result) result.style.display = 'block';
        console.log(t('signatureGenerationComplete'));

        // 他のタブのボタンを有効化
        const verifyBtn = document.getElementById('verifyBtn') as HTMLButtonElement;
        const generateProofBtn = document.getElementById('generateProofBtn') as HTMLButtonElement;
        if (verifyBtn) verifyBtn.disabled = false;
        if (generateProofBtn) generateProofBtn.disabled = false;

        // 署名検証タブの情報を更新
        updateVerifyInfo();

    } catch (error) {
        console.error(t('signatureGenerationFailed'), error);
        alert(t('signatureGenerationFailed') + ': ' + (error as Error).message);
    } finally {
        if (btn) btn.disabled = false;
        if (loading) loading.style.display = 'none';
    }
}

// 署名検証
async function verifySignature(): Promise<void> {
    const btn = document.getElementById('verifyBtn') as HTMLButtonElement;
    const loading = document.getElementById('verifyLoading') as HTMLElement;
    const result = document.getElementById('verifyResult') as HTMLElement;

    try {
        if (btn) btn.disabled = true;
        if (loading) loading.style.display = 'inline-block';

        // 署名を検証
        const isValid = bbs.verify(signature, publicKey, messages);

        // 結果を表示
        if (result) {
            result.className = isValid ? 'result success' : 'result error';
            result.innerHTML = `
                <h3>${t('signatureVerificationResult')}</h3>
                <div class="status ${isValid ? 'ok' : 'ng'}">
                    ${isValid ? t('signatureValid') : t('signatureInvalid')}
                </div>
            `;
            result.style.display = 'block';
        }

        console.log(t('signatureVerificationComplete'), isValid);

    } catch (error) {
        console.error(t('signatureVerificationFailed'), error);
        alert(t('signatureVerificationFailed') + ': ' + (error as Error).message);
    } finally {
        if (btn) btn.disabled = false;
        if (loading) loading.style.display = 'none';
    }
}

// 証明生成
async function generateProof(): Promise<void> {
    const btn = document.getElementById('generateProofBtn') as HTMLButtonElement;
    const loading = document.getElementById('proofLoading') as HTMLElement;
    const result = document.getElementById('proofResult') as HTMLElement;

    try {
        if (btn) btn.disabled = true;
        if (loading) loading.style.display = 'inline-block';

        // 開示する項目を取得
        disclosedIndices = [];
        disclosedMessages = [];
        const fieldNames = ['姓', '名', '性別', '都道府県', '群市町村', '住所', '誕生年', '誕生月', '誕生日'];

        for (let i = 0; i < fieldNames.length; i++) {
            const radio = document.querySelector(`input[name="disclose_${i}"]:checked`) as HTMLInputElement;
            if (radio && radio.value === 'disclose') {
                disclosedIndices.push(i);
                disclosedMessages.push(messages[i]);
            }
        }

        if (disclosedIndices.length === 0) {
            alert(t('atLeastOneItemRequired'));
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
        const proofPreview = document.getElementById('proofPreview');
        if (proofPreview) proofPreview.textContent = getPreview(proofHex);

        if (result) result.style.display = 'block';
        console.log(t('proofGenerationComplete'));

        // 証明検証タブのボタンを有効化
        const verifyProofBtn = document.getElementById('verifyProofBtn') as HTMLButtonElement;
        if (verifyProofBtn) verifyProofBtn.disabled = false;

        // 証明検証タブの情報を更新
        updateProofVerifyInfo();

        // 証明生成タブの情報も更新
        updateProofInfo();

    } catch (error) {
        console.error(t('proofGenerationFailed'), error);
        alert(t('proofGenerationFailed') + ': ' + (error as Error).message);
    } finally {
        if (btn) btn.disabled = false;
        if (loading) loading.style.display = 'none';
    }
}

// 証明検証
async function verifyProof(): Promise<void> {
    const btn = document.getElementById('verifyProofBtn') as HTMLButtonElement;
    const loading = document.getElementById('proofVerifyLoading') as HTMLElement;
    const result = document.getElementById('proofVerifyResult') as HTMLElement;

    try {
        if (btn) btn.disabled = true;
        if (loading) loading.style.display = 'inline-block';

        // nonceが存在するかチェック
        if (!proofNonce) {
            throw new Error(t('proofNotGenerated'));
        }

        // nonce（証明生成時と同じもの）
        const nonce = proofNonce;

        // 証明を検証
        const isValid = bbs.verifyProof(publicKey, proof, disclosedMessages, new Uint32Array(disclosedIndices), nonce);

        // 結果を表示
        if (result) {
            result.className = isValid ? 'result success' : 'result error';
            result.innerHTML = `
                <h3>${t('proofVerificationResult')}</h3>
                <div class="status ${isValid ? 'ok' : 'ng'}">
                    ${isValid ? t('proofValid') : t('proofInvalid')}
                </div>
            `;
            result.style.display = 'block';
        }

        console.log(t('proofVerificationComplete'), isValid);

    } catch (error) {
        console.error(t('proofVerificationFailed'), error);
        alert(t('proofVerificationFailed') + ': ' + (error as Error).message);
    } finally {
        if (btn) btn.disabled = false;
        if (loading) loading.style.display = 'none';
    }
}

// 署名検証タブの情報を更新
function updateVerifyInfo(): void {
    if (messages.length === 0) return;

    const fieldNames = [t('lastName'), t('firstName'), t('gender'), t('prefecture'), t('city'), t('address'), t('birthYear'), t('birthMonth'), t('birthDay')];
    const verifyMessages = document.getElementById('verifyMessages') as HTMLElement;
    const verifyEditControls = document.getElementById('verifyEditControls') as HTMLElement;
    const verifyEditFields = document.getElementById('verifyEditFields') as HTMLElement;
    
    // メッセージ情報を表示
    let html = '';
    messages.forEach((msg, index) => {
        html += `<div><strong>${fieldNames[index]}:</strong> ${uint8ArrayToString(msg)}</div>`;
    });
    if (verifyMessages) verifyMessages.innerHTML = html;

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
    if (verifyEditFields) verifyEditFields.innerHTML = html;
    if (verifyEditControls) verifyEditControls.style.display = 'block';
}

// 証明生成タブの情報を更新
function updateProofInfo(): void {
    if (messages.length === 0) return;

    const fieldNames = [t('lastName'), t('firstName'), t('gender'), t('prefecture'), t('city'), t('address'), t('birthYear'), t('birthMonth'), t('birthDay')];
    const proofMessages = document.getElementById('proofMessages') as HTMLElement;
    const disclosureControls = document.getElementById('disclosureControls') as HTMLElement;
    
    // メッセージ情報を表示
    let html = '';
    messages.forEach((msg, index) => {
        html += `<div><strong>${fieldNames[index]}:</strong> ${uint8ArrayToString(msg)}</div>`;
    });
    if (proofMessages) proofMessages.innerHTML = html;

    // 開示制御を生成
    html = '';
    messages.forEach((msg, index) => {
        const value = uint8ArrayToString(msg);
        html += `
            <div class="disclosure-item">
                <label>
                    <input type="radio" name="disclose_${index}" value="disclose" checked>
                    ${t('disclose')}
                </label>
                <label>
                    <input type="radio" name="disclose_${index}" value="hide">
                    ${t('hide')}
                </label>
                <div class="field-value">${value}</div>
            </div>
        `;
    });
    if (disclosureControls) {
        disclosureControls.innerHTML = html;
        disclosureControls.style.display = 'grid';
    }

    // ラジオボタンの変更イベントを追加
    messages.forEach((msg, index) => {
        const radios = document.querySelectorAll(`input[name="disclose_${index}"]`);
        const fieldValue = disclosureControls?.children[index]?.querySelector('.field-value') as HTMLElement;
        
        radios.forEach(radio => {
            radio.addEventListener('change', function(this: HTMLInputElement) {
                if (this.value === 'hide') {
                    if (fieldValue) {
                        fieldValue.textContent = '***';
                        fieldValue.classList.add('hidden');
                    }
                } else {
                    if (fieldValue) {
                        fieldValue.textContent = uint8ArrayToString(msg);
                        fieldValue.classList.remove('hidden');
                    }
                }
            });
        });
    });
}

// 証明検証タブの情報を更新
function updateProofVerifyInfo(): void {
    if (disclosedMessages.length === 0) return;

    const fieldNames = [t('lastName'), t('firstName'), t('gender'), t('prefecture'), t('city'), t('address'), t('birthYear'), t('birthMonth'), t('birthDay')];
    const proofVerifyMessages = document.getElementById('proofVerifyMessages') as HTMLElement;
    const proofVerifyEditControls = document.getElementById('proofVerifyEditControls') as HTMLElement;
    const proofVerifyEditFields = document.getElementById('proofVerifyEditFields') as HTMLElement;
    
    // 開示メッセージ情報を表示
    let html = '';
    disclosedIndices.forEach((index, i) => {
        html += `<div><strong>${fieldNames[index]}:</strong> ${uint8ArrayToString(disclosedMessages[i])}</div>`;
    });
    if (proofVerifyMessages) proofVerifyMessages.innerHTML = html;

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
    if (proofVerifyEditFields) proofVerifyEditFields.innerHTML = html;
    if (proofVerifyEditControls) proofVerifyEditControls.style.display = 'block';
}

// 署名検証用メッセージを更新
function updateVerifyMessage(index: number, value: string): void {
    if (index >= 0 && index < messages.length) {
        messages[index] = stringToUint8Array(value);
    }
}

// 証明検証用メッセージを更新
function updateProofVerifyMessage(index: number, value: string): void {
    if (index >= 0 && index < disclosedMessages.length) {
        disclosedMessages[index] = stringToUint8Array(value);
    }
}

// 署名検証メッセージをリセット
function resetVerifyMessages(): void {
    if (originalMessages.length > 0) {
        messages = originalMessages.map(msg => new Uint8Array(msg));
        updateVerifyInfo();
    }
}

// 証明検証メッセージをリセット
function resetProofVerifyMessages(): void {
    if (originalDisclosedMessages.length > 0) {
        disclosedMessages = originalDisclosedMessages.map(msg => new Uint8Array(msg));
        updateProofVerifyInfo();
    }
}

// ページ読み込み時の初期化
document.addEventListener('DOMContentLoaded', function() {
    // BBSライブラリを初期化
    initBBS();
    
    // フォームのイベントリスナーを設定
    const signForm = document.getElementById('signForm');
    if (signForm) signForm.addEventListener('submit', generateSignature);
    
    // 言語切り替え機能を初期化（デフォルトで日本語）
    switchLanguage('ja');
});

// グローバルスコープに関数を露出（HTMLのonclick属性から呼び出すため）
(window as any).generateKeys = generateKeys;
(window as any).showTab = showTab;
(window as any).verifySignature = verifySignature;
(window as any).generateProof = generateProof;
(window as any).verifyProof = verifyProof;
(window as any).switchLanguage = switchLanguage;
(window as any).updateVerifyMessage = updateVerifyMessage;
(window as any).updateProofVerifyMessage = updateProofVerifyMessage;
(window as any).resetVerifyMessages = resetVerifyMessages;
(window as any).resetProofVerifyMessages = resetProofVerifyMessages; 