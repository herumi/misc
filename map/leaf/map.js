function reverse([x, y]) {
  if (y < 180) {
    y += 180
  } else {
    y -= 180
  }
  return [-x, y]
}

let g_map;

let g_cur;
let g_rev;

// クリックした地点にマーカーを追加
function onMapClick(e) {
  // 前回のマーカーがあれば削除
  if (g_cur) {
    g_map.removeLayer(g_cur);
    g_map.removeLayer(g_rev);
  }
  console.log({e})
  const x = e.latlng.lat
  const y = e.latlng.lng
  console.log({x,y})
  g_cur = L.marker(e.latlng).bindTooltip("現在地").addTo(g_map);
  g_rev = L.marker(reverse([x, y])).bindTooltip("反対").addTo(g_map);
}

function init() {
  g_map = L.map('mapcontainer', { zoomControl: false });
  g_map.setView([35.40, 136], 2);
  //スケールコントロールを最大幅200px、右下、m単位で地図に追加
  L.control.scale({maxWidth:200,position:'bottomright',imperial:false}).addTo(g_map);

  const gsi =L.tileLayer('https://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png',
    {attribution: "<a href='https://maps.gsi.go.jp/development/ichiran.html' target='_blank'>地理院タイル</a>"});
  //地理院地図の淡色地図タイル
  const gsipale = L.tileLayer('http://cyberjapandata.gsi.go.jp/xyz/pale/{z}/{x}/{y}.png',
    {attribution: "<a href='http://portal.cyberjapan.jp/help/termsofuse.html' target='_blank'>地理院タイル</a>"});
  const baseMaps = {
    "地理院地図" : gsi,
    "淡色地図" : gsipale,
  };

  L.control.layers(baseMaps).addTo(g_map);
  gsi.addTo(g_map);
  g_map.on('click', onMapClick);

  for (let i = 0; i < 360; i += 90) {
    L.marker([0, i]).bindTooltip("東経" + i).addTo(g_map);
  }
}
