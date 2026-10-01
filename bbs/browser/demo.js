/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
let __webpack_exports__ = {};

// @ts-nocheck
// BBS signature demo
/*
  signed messages
  str : octet string (hashed to a scalar)
  int : integer (used for the range proof)
  The birth date is an integer YYYYMMDD so that the order of the integers is the order of the dates.
*/
const FIELDS = [
    { key: 'lastName', kind: 'str' },
    { key: 'firstName', kind: 'str' },
    { key: 'gender', kind: 'str' },
    { key: 'prefecture', kind: 'str' },
    { key: 'city', kind: 'str' },
    { key: 'address', kind: 'str' },
    { key: 'birthDate', kind: 'int' }
];
const BIRTH_IDX = 6;
// the difference of two integers of the form YYYYMMDD is less than 2^25
const BIRTH_BIT_N = 25;
const MAX_AGE = 150;
let bbs = null;
let g_sec = null;
let g_pub = null;
let g_sig = null;
let g_prf = null;
let g_msgs = [];
let g_discIdxs = [];
let g_discMsgs = [];
let g_orgMsgs = [];
let g_orgDiscMsgs = [];
let g_nonce = null;
let g_curLang = 'ja';
let g_selections = [];
// the condition selected in the proof generation tab
const g_ageCond = { useMin: true, minAge: 18, useMax: false, maxAge: 65 };
// the condition of the generated proof (null if the proof has no predicate)
let g_proofAgeCond = null;
// the condition used to verify the proof (editable for testing)
let g_verifyAgeCond = null;
// the reference date of the generated proof
let g_baseDate = null;
const translations = {
    ja: {
        // field names
        lastName: '姓',
        firstName: '名',
        gender: '性別',
        prefecture: '都道府県',
        city: '群市町村',
        address: '住所',
        birthDate: '生年月日',
        // gender
        male: '男',
        female: '女',
        other: 'その他',
        pleaseSelect: '選択してください',
        // disclosure
        disclose: '開示する',
        hide: '開示しない',
        proveAge: '年齢条件だけ証明する',
        ageMinPrefix: '',
        ageMinSuffix: '歳以上',
        ageMaxPrefix: '',
        ageMaxSuffix: '歳以下',
        baseDate: '基準日',
        statement: '証明する内容',
        provenCondition: '証明された条件',
        none: '(なし)',
        ageThresholdMin: '年齢の下限',
        ageThresholdMax: '年齢の上限',
        // messages
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
        atLeastOneItemRequired: '少なくとも1つの項目を開示するか、年齢条件を選ぶ必要があります。',
        ageCondRequired: '年齢条件を少なくとも1つ選んでください。',
        badAge: `年齢は 0 から ${MAX_AGE} の整数で指定してください。`,
        badBirthDate: '生年月日が正しくありません。',
        predicateNotSatisfied: '証明を生成できません。生年月日が年齢条件を満たしていません。',
        proofNotGenerated: '証明が生成されていません。先に証明を生成してください。',
        // results
        signatureValid: 'OK - 署名は有効です',
        signatureInvalid: 'NG - 署名は無効です',
        proofValid: 'OK - 証明は有効です',
        proofInvalid: 'NG - 証明は無効です',
        proofSize: '証明サイズ',
        proofSizeWithoutPred: '年齢条件なしの場合',
        bytes: 'バイト',
        generationTime: '生成時間',
        verificationTime: '検証時間',
        // titles
        signatureVerificationResult: '署名検証結果',
        proofVerificationResult: '証明検証結果'
    },
    en: {
        // field names
        lastName: 'Last Name',
        firstName: 'First Name',
        gender: 'Gender',
        prefecture: 'Prefecture',
        city: 'City',
        address: 'Address',
        birthDate: 'Birth Date',
        // gender
        male: 'Male',
        female: 'Female',
        other: 'Other',
        pleaseSelect: 'Please select',
        // disclosure
        disclose: 'Disclose',
        hide: 'Hide',
        proveAge: 'Prove only the age condition',
        ageMinPrefix: 'Age at least',
        ageMinSuffix: '',
        ageMaxPrefix: 'Age at most',
        ageMaxSuffix: '',
        baseDate: 'Reference date',
        statement: 'Statement to prove',
        provenCondition: 'Proven condition',
        none: '(none)',
        ageThresholdMin: 'Minimum age',
        ageThresholdMax: 'Maximum age',
        // messages
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
        atLeastOneItemRequired: 'Disclose at least one item or select an age condition.',
        ageCondRequired: 'Select at least one age condition.',
        badAge: `The age must be an integer from 0 to ${MAX_AGE}.`,
        badBirthDate: 'The birth date is invalid.',
        predicateNotSatisfied: 'The proof can not be generated. The birth date does not satisfy the age condition.',
        proofNotGenerated: 'Proof has not been generated. Please generate a proof first.',
        // results
        signatureValid: 'OK - Signature is valid',
        signatureInvalid: 'NG - Signature is invalid',
        proofValid: 'OK - Proof is valid',
        proofInvalid: 'NG - Proof is invalid',
        proofSize: 'Proof size',
        proofSizeWithoutPred: 'without the age condition',
        bytes: 'bytes',
        generationTime: 'Generation time',
        verificationTime: 'Verification time',
        // titles
        signatureVerificationResult: 'Signature Verification Result',
        proofVerificationResult: 'Proof Verification Result'
    }
};
function switchLanguage(lang) {
    g_curLang = lang;
    const langJa = document.getElementById('langJa');
    const langEn = document.getElementById('langEn');
    if (langJa)
        langJa.classList.toggle('active', lang === 'ja');
    if (langEn)
        langEn.classList.toggle('active', lang === 'en');
    document.documentElement.lang = lang;
    const title = document.querySelector('title');
    if (title) {
        title.textContent = title.getAttribute(`data-${lang}`);
    }
    // update the text of all elements which have data-ja and data-en
    const elements = document.querySelectorAll('[data-ja][data-en]');
    elements.forEach(element => {
        const text = element.getAttribute(`data-${lang}`);
        if (text) {
            element.textContent = text;
        }
    });
    updateDynamicContent();
}
// update the contents generated by this script
function updateDynamicContent() {
    if (g_msgs.length > 0) {
        updateVerifyInfo();
        updateProofInfo();
    }
    if (g_prf) {
        updateProofVerifyInfo();
    }
}
function t(key) {
    const s = translations[g_curLang][key];
    return s === undefined ? key : s;
}
function fieldName(index) {
    return t(FIELDS[index].key);
}
async function initBBS() {
    try {
        bbs = window.bbs;
        await bbs.init();
        console.log('BBS library is initialized');
    }
    catch (error) {
        console.error('BBS library initialization failed:', error);
        alert(t('bbsInitFailed'));
    }
}
function stringToUint8Array(str) {
    const encoder = new TextEncoder();
    return encoder.encode(str);
}
function uint8ArrayToString(arr) {
    const decoder = new TextDecoder('utf-8');
    return decoder.decode(arr);
}
function escapeHtml(s) {
    return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;').replace(/'/g, '&#39;');
}
// integer YYYYMMDD
function ymd(y, m, d) {
    return BigInt(y * 10000 + m * 100 + d);
}
// 19960320n -> '1996/03/20'
function formatYmd(v) {
    const s = v.toString().padStart(8, '0');
    const n = s.length;
    return `${s.substring(0, n - 4)}/${s.substring(n - 4, n - 2)}/${s.substring(n - 2)}`;
}
function today() {
    const now = new Date();
    return { y: now.getFullYear(), m: now.getMonth() + 1, d: now.getDate() };
}
// string to show a message
function msgToString(msg, index) {
    if (FIELDS[index].kind === 'int')
        return formatYmd(msg);
    return uint8ArrayToString(msg);
}
// string to edit a message
function msgToEditString(msg, index) {
    if (FIELDS[index].kind === 'int')
        return msg.toString();
    return uint8ArrayToString(msg);
}
// message made from the string of an edit field
function editStringToMsg(value, index) {
    if (FIELDS[index].kind === 'int') {
        return /^[0-9]{1,18}$/.test(value) ? BigInt(value) : 0n;
    }
    return stringToUint8Array(value);
}
function cloneMsgs(msgs) {
    return msgs.map(msg => typeof msg === 'bigint' ? msg : new Uint8Array(msg));
}
function isValidAge(age) {
    return Number.isInteger(age) && age >= 0 && age <= MAX_AGE;
}
/*
  predicates for the condition of the age on the reference date
  age >= N : birthDate <= ymd(Y - N, M, D)
  age <= N : the person is not N + 1 years old yet, so birthDate > ymd(Y - N - 1, M, D)
  The bounds are compared as integers, so they need not be real dates.
  The order of the predicates must be the same in the generation and the verification.
*/
function makeAgePreds(cond, base) {
    const preds = [];
    if (cond.useMin) {
        preds.push({ idx: BIRTH_IDX, type: bbs.PRED_LE, bound: ymd(base.y - cond.minAge, base.m, base.d), bitN: BIRTH_BIT_N });
    }
    if (cond.useMax) {
        preds.push({ idx: BIRTH_IDX, type: bbs.PRED_GE, bound: ymd(base.y - cond.maxAge - 1, base.m, base.d) + 1n, bitN: BIRTH_BIT_N });
    }
    return preds;
}
// '18 歳以上' or 'Age at least 18'
function ageText(prefix, age, suffix) {
    return [prefix, String(age), suffix].filter(s => s !== '').join(' ');
}
// lines to explain the predicates
function describeAgeCond(cond, base) {
    const lines = [];
    if (cond.useMin) {
        const bound = ymd(base.y - cond.minAge, base.m, base.d);
        lines.push(`${t('birthDate')} ≤ ${formatYmd(bound)} (${ageText(t('ageMinPrefix'), cond.minAge, t('ageMinSuffix'))})`);
    }
    if (cond.useMax) {
        const bound = ymd(base.y - cond.maxAge - 1, base.m, base.d) + 1n;
        lines.push(`${t('birthDate')} ≥ ${formatYmd(bound)} (${ageText(t('ageMaxPrefix'), cond.maxAge, t('ageMaxSuffix'))})`);
    }
    return lines;
}
function isValidAgeCond(cond) {
    return (!cond.useMin || isValidAge(cond.minAge)) && (!cond.useMax || isValidAge(cond.maxAge));
}
// use the current time as a nonce (YYYYMMDDHHMMSS.mmmm)
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
    return stringToUint8Array(timestamp);
}
// the last 32 bytes of data in hex
function getPreview(data) {
    return '...' + data.substring(data.length - 64);
}
function showTab(tabName) {
    const tabContents = document.querySelectorAll('.tab-content');
    tabContents.forEach(content => content.classList.remove('active'));
    const tabs = document.querySelectorAll('.tab');
    tabs.forEach(tab => tab.classList.remove('active'));
    const targetTab = document.getElementById(tabName);
    if (targetTab)
        targetTab.classList.add('active');
    if (event && event.target) {
        event.target.classList.add('active');
    }
    if (tabName === 'sign' && g_msgs.length > 0) {
        updateVerifyInfo();
        updateProofInfo();
    }
    else if (tabName === 'verify' && g_msgs.length > 0) {
        updateVerifyInfo();
    }
    else if (tabName === 'proof' && g_msgs.length > 0) {
        updateProofInfo();
    }
    else if (tabName === 'proof-verify' && g_prf) {
        updateProofVerifyInfo();
    }
}
async function generateKeys() {
    const btn = document.getElementById('generateKeys');
    const loading = document.getElementById('keygenLoading');
    const result = document.getElementById('keygenResult');
    try {
        if (btn)
            btn.disabled = true;
        if (loading)
            loading.style.display = 'inline-block';
        g_sec = new bbs.SecretKey();
        g_sec.init();
        g_pub = g_sec.getPublicKey();
        const secretKeyHex = g_sec.serializeToHexStr();
        const publicKeyHex = g_pub.serializeToHexStr();
        const secretKeyPreview = document.getElementById('secretKeyPreview');
        const publicKeyPreview = document.getElementById('publicKeyPreview');
        if (secretKeyPreview)
            secretKeyPreview.textContent = getPreview(secretKeyHex);
        if (publicKeyPreview)
            publicKeyPreview.textContent = getPreview(publicKeyHex);
        if (result)
            result.style.display = 'block';
        console.log(t('keyGenerationComplete'));
    }
    catch (error) {
        console.error(t('keyGenerationFailed'), error);
        alert(t('keyGenerationFailed') + ': ' + error.message);
    }
    finally {
        if (btn)
            btn.disabled = false;
        if (loading)
            loading.style.display = 'none';
    }
}
async function generateSignature(event) {
    event.preventDefault();
    const btn = document.getElementById('signBtn');
    const loading = document.getElementById('signLoading');
    const result = document.getElementById('signResult');
    try {
        if (btn)
            btn.disabled = true;
        if (loading)
            loading.style.display = 'inline-block';
        const getValue = (id) => document.getElementById(id)?.value || '';
        const birthYear = Number(getValue('birthYear'));
        const birthMonth = Number(getValue('birthMonth'));
        const birthDay = Number(getValue('birthDay'));
        if (!(Number.isInteger(birthYear) && birthYear >= 1 && birthYear <= 9999 && Number.isInteger(birthMonth) && birthMonth >= 1 && birthMonth <= 12 && Number.isInteger(birthDay) && birthDay >= 1 && birthDay <= 31)) {
            alert(t('badBirthDate'));
            return;
        }
        // the order is the same as FIELDS
        g_msgs = [
            stringToUint8Array(getValue('lastName')),
            stringToUint8Array(getValue('firstName')),
            stringToUint8Array(getValue('gender')),
            stringToUint8Array(getValue('prefecture')),
            stringToUint8Array(getValue('city')),
            stringToUint8Array(getValue('address')),
            ymd(birthYear, birthMonth, birthDay)
        ];
        g_orgMsgs = cloneMsgs(g_msgs);
        // disclose all messages by default
        g_selections = new Array(g_msgs.length).fill('disclose');
        g_sig = bbs.sign(g_sec, g_pub, g_msgs);
        const signatureHex = g_sig.serializeToHexStr();
        const signaturePreview = document.getElementById('signaturePreview');
        if (signaturePreview)
            signaturePreview.textContent = getPreview(signatureHex);
        if (result)
            result.style.display = 'block';
        console.log(t('signatureGenerationComplete'));
        const verifyBtn = document.getElementById('verifyBtn');
        const generateProofBtn = document.getElementById('generateProofBtn');
        if (verifyBtn)
            verifyBtn.disabled = false;
        if (generateProofBtn)
            generateProofBtn.disabled = false;
        updateVerifyInfo();
    }
    catch (error) {
        console.error(t('signatureGenerationFailed'), error);
        alert(t('signatureGenerationFailed') + ': ' + error.message);
    }
    finally {
        if (btn)
            btn.disabled = false;
        if (loading)
            loading.style.display = 'none';
    }
}
async function verifySignature() {
    const btn = document.getElementById('verifyBtn');
    const loading = document.getElementById('verifyLoading');
    const result = document.getElementById('verifyResult');
    try {
        if (btn)
            btn.disabled = true;
        if (loading)
            loading.style.display = 'inline-block';
        const isValid = bbs.verify(g_sig, g_pub, g_msgs);
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
    }
    catch (error) {
        console.error(t('signatureVerificationFailed'), error);
        alert(t('signatureVerificationFailed') + ': ' + error.message);
    }
    finally {
        if (btn)
            btn.disabled = false;
        if (loading)
            loading.style.display = 'none';
    }
}
async function generateProof() {
    const btn = document.getElementById('generateProofBtn');
    const loading = document.getElementById('proofLoading');
    const result = document.getElementById('proofResult');
    try {
        if (btn)
            btn.disabled = true;
        if (loading)
            loading.style.display = 'inline-block';
        const discIdxs = [];
        const discMsgs = [];
        for (let i = 0; i < g_msgs.length; i++) {
            if (g_selections[i] === 'disclose') {
                discIdxs.push(i);
                discMsgs.push(g_msgs[i]);
            }
        }
        // predicates for the age
        const base = today();
        let preds = [];
        if (g_selections[BIRTH_IDX] === 'predicate') {
            if (!g_ageCond.useMin && !g_ageCond.useMax) {
                alert(t('ageCondRequired'));
                return;
            }
            if (!isValidAgeCond(g_ageCond)) {
                alert(t('badAge'));
                return;
            }
            preds = makeAgePreds(g_ageCond, base);
        }
        if (discIdxs.length === 0 && preds.length === 0) {
            alert(t('atLeastOneItemRequired'));
            return;
        }
        // the nonce is bound to the proof as the presentation header
        const nonce = generateTimestampNonce();
        const idxs = new Uint32Array(discIdxs);
        const begin = performance.now();
        let prf;
        if (preds.length > 0) {
            try {
                prf = bbs.proofGenEx(g_pub, g_sig, g_msgs, idxs, preds, undefined, nonce);
            }
            catch (error) {
                // the library refuses to make a proof of a false statement
                console.error(t('proofGenerationFailed'), error);
                alert(t('predicateNotSatisfied'));
                return;
            }
        }
        else {
            prf = bbs.proofGen(g_pub, g_sig, g_msgs, idxs, undefined, nonce);
        }
        const msec = performance.now() - begin;
        g_prf = prf;
        g_nonce = nonce;
        g_discIdxs = discIdxs;
        g_discMsgs = discMsgs;
        g_orgDiscMsgs = cloneMsgs(discMsgs);
        g_baseDate = base;
        g_proofAgeCond = preds.length > 0 ? { ...g_ageCond } : null;
        g_verifyAgeCond = g_proofAgeCond ? { ...g_proofAgeCond } : null;
        const proofHex = bbs.toHexStr(g_prf);
        const proofPreview = document.getElementById('proofPreview');
        if (proofPreview) {
            const nonceStr = uint8ArrayToString(g_nonce);
            proofPreview.textContent = `Nonce: ${nonceStr.substring(0, 20)}... | Proof: ${getPreview(proofHex)}`;
        }
        const proofStats = document.getElementById('proofStats');
        if (proofStats) {
            let s = `${t('proofSize')}: ${g_prf.length} ${t('bytes')}`;
            if (preds.length > 0) {
                const baseSize = bbs.getProofSize(g_msgs.length - discIdxs.length);
                s += ` (${t('proofSizeWithoutPred')}: ${baseSize} ${t('bytes')})`;
            }
            s += ` / ${t('generationTime')}: ${msec.toFixed(1)} ms`;
            proofStats.textContent = s;
        }
        if (result)
            result.style.display = 'block';
        console.log(t('proofGenerationComplete'), discIdxs, preds);
        const verifyProofBtn = document.getElementById('verifyProofBtn');
        if (verifyProofBtn)
            verifyProofBtn.disabled = false;
        // the result of the previous verification is obsolete
        const proofVerifyResult = document.getElementById('proofVerifyResult');
        if (proofVerifyResult)
            proofVerifyResult.style.display = 'none';
        updateProofVerifyInfo();
    }
    catch (error) {
        console.error(t('proofGenerationFailed'), error);
        alert(t('proofGenerationFailed') + ': ' + error.message);
    }
    finally {
        if (btn)
            btn.disabled = false;
        if (loading)
            loading.style.display = 'none';
    }
}
async function verifyProof() {
    const btn = document.getElementById('verifyProofBtn');
    const loading = document.getElementById('proofVerifyLoading');
    const result = document.getElementById('proofVerifyResult');
    try {
        if (btn)
            btn.disabled = true;
        if (loading)
            loading.style.display = 'inline-block';
        if (!g_prf || !g_nonce) {
            throw new Error(t('proofNotGenerated'));
        }
        const idxs = new Uint32Array(g_discIdxs);
        const begin = performance.now();
        let isValid = false;
        if (g_verifyAgeCond && g_baseDate) {
            // the predicates are made from the (maybe edited) condition
            if (isValidAgeCond(g_verifyAgeCond)) {
                const preds = makeAgePreds(g_verifyAgeCond, g_baseDate);
                isValid = bbs.proofVerifyEx(g_pub, g_prf, g_discMsgs, idxs, preds, undefined, g_nonce);
            }
        }
        else {
            isValid = bbs.proofVerify(g_pub, g_prf, g_discMsgs, idxs, undefined, g_nonce);
        }
        const msec = performance.now() - begin;
        if (result) {
            result.className = isValid ? 'result success' : 'result error';
            result.innerHTML = `
                <h3>${t('proofVerificationResult')}</h3>
                <div class="status ${isValid ? 'ok' : 'ng'}">
                    ${isValid ? t('proofValid') : t('proofInvalid')}
                </div>
                <div class="stats">${t('verificationTime')}: ${msec.toFixed(1)} ms</div>
            `;
            result.style.display = 'block';
        }
        console.log(t('proofVerificationComplete'), isValid);
    }
    catch (error) {
        console.error(t('proofVerificationFailed'), error);
        alert(t('proofVerificationFailed') + ': ' + error.message);
    }
    finally {
        if (btn)
            btn.disabled = false;
        if (loading)
            loading.style.display = 'none';
    }
}
// html of the list of messages
function makeMsgListHtml(msgs, idxs) {
    if (msgs.length === 0)
        return `<div>${t('none')}</div>`;
    let html = '';
    msgs.forEach((msg, i) => {
        html += `<div><strong>${fieldName(idxs[i])}:</strong> ${escapeHtml(msgToString(msg, idxs[i]))}</div>`;
    });
    return html;
}
function allIdxs() {
    return FIELDS.map((_, i) => i);
}
// update the signature verification tab
function updateVerifyInfo() {
    if (g_msgs.length === 0)
        return;
    const verifyMessages = document.getElementById('verifyMessages');
    const verifyEditControls = document.getElementById('verifyEditControls');
    const verifyEditFields = document.getElementById('verifyEditFields');
    if (verifyMessages)
        verifyMessages.innerHTML = makeMsgListHtml(g_msgs, allIdxs());
    // fields to edit the messages
    let html = '';
    g_msgs.forEach((msg, index) => {
        const isInt = FIELDS[index].kind === 'int';
        html += `
            <div class="edit-field">
                <label for="verify_edit_${index}">${fieldName(index)}${isInt ? ' (YYYYMMDD)' : ''}</label>
                <input type="${isInt ? 'number' : 'text'}" id="verify_edit_${index}" value="${escapeHtml(msgToEditString(msg, index))}"
                       onchange="updateVerifyMessage(${index}, this.value)">
            </div>
        `;
    });
    if (verifyEditFields)
        verifyEditFields.innerHTML = html;
    if (verifyEditControls)
        verifyEditControls.style.display = 'block';
}
// text of the value of a message in the proof generation tab
function selectionValueText(index) {
    return g_selections[index] === 'disclose' ? msgToString(g_msgs[index], index) : '***';
}
// update the description of the age condition in the proof generation tab
function updateAgeStatement() {
    const ageCond = document.getElementById('ageCond');
    const baseDate = document.getElementById('ageBaseDate');
    const statement = document.getElementById('ageStatement');
    if (ageCond)
        ageCond.style.display = g_selections[BIRTH_IDX] === 'predicate' ? 'block' : 'none';
    const base = today();
    if (baseDate)
        baseDate.textContent = formatYmd(ymd(base.y, base.m, base.d));
    if (statement) {
        if (!isValidAgeCond(g_ageCond)) {
            statement.textContent = t('badAge');
        }
        else {
            const lines = describeAgeCond(g_ageCond, base);
            statement.innerHTML = lines.length > 0 ? lines.map(escapeHtml).join('<br>') : t('none');
        }
    }
}
// update the proof generation tab
function updateProofInfo() {
    if (g_msgs.length === 0)
        return;
    const proofMessages = document.getElementById('proofMessages');
    const disclosureControls = document.getElementById('disclosureControls');
    if (proofMessages)
        proofMessages.innerHTML = makeMsgListHtml(g_msgs, allIdxs());
    // controls to select how to show each message
    let html = '';
    g_msgs.forEach((msg, index) => {
        const sel = g_selections[index];
        const radio = (value, label) => `
                <label>
                    <input type="radio" name="disclose_${index}" value="${value}" ${sel === value ? 'checked' : ''}>
                    ${label}
                </label>`;
        html += `
            <div class="disclosure-item${index === BIRTH_IDX ? ' wide' : ''}">
                <h4>${fieldName(index)}</h4>
                ${radio('disclose', t('disclose'))}
                ${radio('hide', t('hide'))}`;
        if (index === BIRTH_IDX) {
            html += `
                ${radio('predicate', t('proveAge'))}
                <div id="ageCond" class="age-cond">
                    <label>
                        <input type="checkbox" id="ageMinUse" ${g_ageCond.useMin ? 'checked' : ''}>
                        ${t('ageMinPrefix')}
                        <input type="number" id="ageMin" min="0" max="${MAX_AGE}" value="${g_ageCond.minAge}">
                        ${t('ageMinSuffix')}
                    </label>
                    <label>
                        <input type="checkbox" id="ageMaxUse" ${g_ageCond.useMax ? 'checked' : ''}>
                        ${t('ageMaxPrefix')}
                        <input type="number" id="ageMax" min="0" max="${MAX_AGE}" value="${g_ageCond.maxAge}">
                        ${t('ageMaxSuffix')}
                    </label>
                    <div>${t('baseDate')}: <span id="ageBaseDate"></span></div>
                    <div>${t('statement')}:</div>
                    <div id="ageStatement" class="age-statement"></div>
                </div>`;
        }
        const hidden = sel !== 'disclose';
        html += `
                <div class="field-value ${hidden ? 'hidden' : ''}">${escapeHtml(selectionValueText(index))}</div>
            </div>
        `;
    });
    if (disclosureControls) {
        disclosureControls.innerHTML = html;
        disclosureControls.style.display = 'grid';
    }
    g_msgs.forEach((msg, index) => {
        const radios = document.querySelectorAll(`input[name="disclose_${index}"]`);
        const fieldValue = disclosureControls?.children[index]?.querySelector('.field-value');
        radios.forEach(radio => {
            radio.addEventListener('change', function () {
                g_selections[index] = this.value;
                if (fieldValue) {
                    fieldValue.textContent = selectionValueText(index);
                    fieldValue.classList.toggle('hidden', g_selections[index] !== 'disclose');
                }
                if (index === BIRTH_IDX)
                    updateAgeStatement();
            });
        });
    });
    // inputs of the age condition
    const bind = (id, handler) => {
        const e = document.getElementById(id);
        if (!e)
            return;
        e.addEventListener('input', () => {
            handler(e);
            updateAgeStatement();
        });
    };
    bind('ageMinUse', e => { g_ageCond.useMin = e.checked; });
    bind('ageMaxUse', e => { g_ageCond.useMax = e.checked; });
    bind('ageMin', e => { g_ageCond.minAge = e.value === '' ? NaN : Number(e.value); });
    bind('ageMax', e => { g_ageCond.maxAge = e.value === '' ? NaN : Number(e.value); });
    updateAgeStatement();
}
// update the description of the proven condition in the proof verification tab
function updateProofVerifyPreds() {
    const proofVerifyPreds = document.getElementById('proofVerifyPreds');
    if (!proofVerifyPreds)
        return;
    if (!g_verifyAgeCond || !g_baseDate) {
        proofVerifyPreds.innerHTML = `<div>${t('none')}</div>`;
        return;
    }
    let html = '';
    if (!isValidAgeCond(g_verifyAgeCond)) {
        html += `<div>${t('badAge')}</div>`;
    }
    else {
        describeAgeCond(g_verifyAgeCond, g_baseDate).forEach(line => {
            html += `<div>${escapeHtml(line)}</div>`;
        });
    }
    html += `<div>${t('baseDate')}: ${formatYmd(ymd(g_baseDate.y, g_baseDate.m, g_baseDate.d))}</div>`;
    proofVerifyPreds.innerHTML = html;
}
// update the proof verification tab
function updateProofVerifyInfo() {
    if (!g_prf)
        return;
    const proofVerifyMessages = document.getElementById('proofVerifyMessages');
    const proofVerifyEditControls = document.getElementById('proofVerifyEditControls');
    const proofVerifyEditFields = document.getElementById('proofVerifyEditFields');
    if (proofVerifyMessages)
        proofVerifyMessages.innerHTML = makeMsgListHtml(g_discMsgs, g_discIdxs);
    updateProofVerifyPreds();
    // fields to edit the disclosed messages and the thresholds of the age
    let html = '';
    g_discIdxs.forEach((index, i) => {
        const isInt = FIELDS[index].kind === 'int';
        html += `
            <div class="edit-field">
                <label for="proof_verify_edit_${i}">${fieldName(index)}${isInt ? ' (YYYYMMDD)' : ''}</label>
                <input type="${isInt ? 'number' : 'text'}" id="proof_verify_edit_${i}" value="${escapeHtml(msgToEditString(g_discMsgs[i], index))}"
                       onchange="updateProofVerifyMessage(${i}, this.value)">
            </div>
        `;
    });
    if (g_verifyAgeCond) {
        const ageField = (which, label, value) => `
            <div class="edit-field">
                <label for="proof_verify_age_${which}">${label}</label>
                <input type="number" id="proof_verify_age_${which}" min="0" max="${MAX_AGE}" value="${Number.isNaN(value) ? '' : value}"
                       oninput="updateProofVerifyAge('${which}', this.value)">
            </div>
        `;
        if (g_verifyAgeCond.useMin)
            html += ageField('min', t('ageThresholdMin'), g_verifyAgeCond.minAge);
        if (g_verifyAgeCond.useMax)
            html += ageField('max', t('ageThresholdMax'), g_verifyAgeCond.maxAge);
    }
    if (proofVerifyEditFields)
        proofVerifyEditFields.innerHTML = html;
    if (proofVerifyEditControls)
        proofVerifyEditControls.style.display = 'block';
}
function updateVerifyMessage(index, value) {
    if (index >= 0 && index < g_msgs.length) {
        g_msgs[index] = editStringToMsg(value, index);
    }
}
function updateProofVerifyMessage(index, value) {
    if (index >= 0 && index < g_discMsgs.length) {
        g_discMsgs[index] = editStringToMsg(value, g_discIdxs[index]);
    }
}
// change the threshold of the age used in the verification
function updateProofVerifyAge(which, value) {
    if (!g_verifyAgeCond)
        return;
    const age = value === '' ? NaN : Number(value);
    if (which === 'min') {
        g_verifyAgeCond.minAge = age;
    }
    else {
        g_verifyAgeCond.maxAge = age;
    }
    updateProofVerifyPreds();
}
function resetVerifyMessages() {
    if (g_orgMsgs.length > 0) {
        g_msgs = cloneMsgs(g_orgMsgs);
        updateVerifyInfo();
    }
}
function resetProofVerifyMessages() {
    if (!g_prf)
        return;
    g_discMsgs = cloneMsgs(g_orgDiscMsgs);
    g_verifyAgeCond = g_proofAgeCond ? { ...g_proofAgeCond } : null;
    updateProofVerifyInfo();
}
document.addEventListener('DOMContentLoaded', function () {
    // expose the functions called from the attributes of HTML
    window.generateKeys = generateKeys;
    window.showTab = showTab;
    window.verifySignature = verifySignature;
    window.generateProof = generateProof;
    window.verifyProof = verifyProof;
    window.switchLanguage = switchLanguage;
    window.updateVerifyMessage = updateVerifyMessage;
    window.updateProofVerifyMessage = updateProofVerifyMessage;
    window.updateProofVerifyAge = updateProofVerifyAge;
    window.resetVerifyMessages = resetVerifyMessages;
    window.resetProofVerifyMessages = resetProofVerifyMessages;
    initBBS();
    const signForm = document.getElementById('signForm');
    if (signForm)
        signForm.addEventListener('submit', generateSignature);
    switchLanguage('ja');
});

window.demo = __webpack_exports__;
/******/ })()
;