// --- Inspiration ---
function getInspiration() {
  fetch('/cgi-bin/recipe.py')
    .then(res => res.text())
    .then(html => document.getElementById('recipeResult').innerHTML = html)
    .catch(err => {
      document.getElementById('recipeResult').innerHTML = "Failed to get recipe 😢";
      console.error(err);
    });
}

// --- Show / Hide Info ---
function showInfo(msg) {
  const box = document.getElementById('infoBox');
  box.innerHTML = `${msg} <button onclick="hideInfo()">✖</button>`;
  box.classList.add('show');
}

function hideInfo() {
  const box = document.getElementById('infoBox');
  box.classList.remove('show');
}

// ---- add items to list
function addtoList() {
  fetch("/cgi-bin/add_to_list.php") 
    .then(res => res.text())
    .then(html => {
      document.getElementById("shoppingListContainer").innerHTML = html;
    })
    .catch(err => {
      console.error("Error loading list:", err);
    });
}


function addIngredients(ingredients) {
  // uploadId muss global verfügbar sein
  const url = `/cgi-bin/add_to_list.php?uploadId=${encodeURIComponent(uploadId)}`;

  fetch(url, {
    method: "POST",
    headers: { "Content-Type": "application/x-www-form-urlencoded" },
    body: new URLSearchParams({ items: ingredients }).toString()
	})
	.then(res => res.text())
	.then(msg => {
		// showInfo(msg);
		loadShoppingList();
	})
  .catch(err => {
    console.error("Error loading list:", err);
  });
}
let uploadId = null;
async function initUpload() {
	try {
		const res = await fetch("/getUploadId");
		if (!res.ok) throw new Error("Could not get uploadId");
		const data = await res.json();
		uploadId = data.uploadId;
		console.log("Upload ID:", uploadId);
	} catch (err) {
		console.error(err);
	}
}

async function loadShoppingList() {
  try {
    const port = window.location.port;
    const res = await fetch(`uploads/${uploadId}/${uploadId}.txt`);
    
    	if (!res.ok) {
	      const html = await res.text();
		    // ersetzt das aktuelle Dokument mit dem empfangenen HTML
	      document.open();
	      document.write(html);
	      document.close();
	    }
    
    //if (!res.ok) throw new Error("Could not load shopping list");
    const text = await res.text();

    const items = text.trim().split('\n').filter(line => line.length > 0);
    const list = document.getElementById('itemList');
    list.innerHTML = '';

    items.forEach(item => {
      const li = document.createElement('li');
      li.className = 'shopping-item';
      li.innerHTML = `
        <input type="checkbox">
        <span>${item}</span>
        <button class="delete-item" onclick="deleteItem('${item}')">
          <i class="fa fa-trash" aria-hidden="true"></i>
        </button>
      `;
      list.appendChild(li);
    });
  } catch (err) {
    console.error(err);
  }
}

// --- Delete Item ---
async function deleteItem(name) {
  try {
    const res = await fetch(`uploads/${uploadId}/${uploadId}.txt`, {
      method: 'DELETE',
      headers: { 'Content-Type': 'text/plain' },
      body: name
    });
    if (!res.ok) {
	    const html = await res.text();
		  // ersetzt das aktuelle Dokument mit dem empfangenen HTML
	    document.open();
	    document.write(html);
	    document.close();
	  }


    else if (res.ok) loadShoppingList();
    else console.error("Delete failed", res.status);
  } catch (err) {
    console.error("Delete error:", err);
  }
}

function updateDeleteBgButton() {
  const btn = document.getElementById('deleteBgBtn');
  const savedBg = localStorage.getItem('backgroundImage');
  if (savedBg) {
    btn.style.display = 'inline-block'; // Button sichtbar
  } else {
    btn.style.display = 'none'; // Button verstecken
  }
}

function initDeleteBgButton() {
  const btn = document.getElementById('deleteBgBtn');
  if (!btn) return;

  btn.addEventListener('click', async function() {
	console.log('DELETE button clicked; uploadId=', uploadId);
    if (!uploadId) {
      showInfo("No uploadId set — cannot delete background.");
      return;
    }

    if (!confirm("Delete all background images for this upload?")) return;

    try {
      const res = await fetch(`uploads/${uploadId}/files`, {
        method: 'DELETE'
      });
	  
	  console.log('CONSOLE LOG DELETE response:', res.status, res.ok, res.headers.get('content-type'));

      if (res.status === 405) {
		console.log('Found correct status 405');
        showInfo("DELETE not allowed!");
		return;
      } 

	  if (!res.ok) {
		const text = await res.text();
		console.error("Delete failed:", res.status, text);
		showInfo("Delete failed (" + res.status + ")");
		return;
	  }
	  showInfo("Background images deleted.");
	  setBackground(null);
	  updateDeleteBgButton()
    } catch (err) {
      console.error("Delete error:", err);
      showInfo("Delete error — see console");
    }
  });
}
document.getElementById("addItemForm").addEventListener("submit", async function(e) {
  e.preventDefault();
  const port = window.location.port;
  const formData = new FormData(this);
  const body = new URLSearchParams(formData).toString();

  try {
    const res = await fetch(`uploads/${uploadId}/${uploadId}.txt`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body
    });

    if (res.status === 405) {
      const data = await res.json();
      showInfo(data.error);
    } else if (res.ok) {
      loadShoppingList();
      this.reset();
    } else console.error("POST failed", res.status);
  } catch (err) {
    console.error("POST error:", err);
  }
});

function setBackground(imageFile) {
 
  const port = window.location.port;
  const body = document.body;

  if (imageFile) {
    // 🔥 speichere gewählten Hintergrund
    localStorage.setItem('backgroundImage', imageFile);
    body.style.backgroundImage = `url('uploads/${uploadId}/files/${imageFile}')`;
    body.style.backgroundColor = "transparent";
  } else {
    localStorage.removeItem('backgroundImage');
    body.style.backgroundImage = "none";
    body.style.backgroundColor = "rgb(27, 46, 60)";
  }

  // Padding bleibt wie gehabt
  body.style.backgroundSize = "cover";
  body.style.backgroundPosition = "center";
  body.style.minHeight = "100vh";
  updateDeleteBgButton();
}


// --- Upload Button ---
function initBackgroundUpload() {
  const uploadBtn = document.getElementById('uploadBtn');
  const fileInput = document.getElementById('fileInput');

  uploadBtn.addEventListener('click', () => fileInput.click());

  fileInput.addEventListener('change', async function() {
    const file = this.files[0];
    if (!file) return;

    const port = window.location.port;
    const formData = new FormData();
    formData.append('file', file);

    try {
      const res = await fetch(`uploads/${uploadId}/files`, { method: 'POST', body: formData });
      if (res.ok) {
        showInfo(`✅ ${file.name} uploaded successfully!`);
        // 🔥 Sofort neues Hintergrundbild setzen
        setBackground(file.name);
      } else {
        showInfo(`Upload failed (${res.status})`);
      }
    } catch (err) {
      console.error(err);
      showInfo("Upload error — see console");
    } finally {
      this.value = "";
    }
  });
}

document.getElementById('deleteBgBtn').addEventListener('click', () => {
  setBackground(null);       // Setzt Standard-Hintergrund
  updateDeleteBgButton();    // Button wieder ausblenden
});

async function backgroundExists(filename) {
  try {
    const res = await fetch(`/uploads/${uploadId}/files/`);
	// const res = await fetch(`/uploads/${uploadId}/files`);
    if (!res.ok) return false;
    const arr = await res.json();
    return arr.indexOf(filename) !== -1;
  } catch (e) {
    console.error("Error checking background existence:", e);
    return false;
  }
}

window.addEventListener('DOMContentLoaded', async() => {

  await initUpload(); 

  initBackgroundUpload();
  initDeleteBgButton();
  loadShoppingList();

  const savedBg = localStorage.getItem('backgroundImage');
  if (savedBg) {
    const ok = await backgroundExists(savedBg);
    if (ok) {
      setBackground(savedBg);
    } else {
      // Datei nicht vorhanden — aufräumen und Default anzeigen
      localStorage.removeItem('backgroundImage');
    }
	// setBackground(savedBg);
  }
});

function trigger405() {
  fetch("/index.html", { method: "DELETE" })
    .then(res => {
      if (!res.ok) {
        res.text().then(html => {
          document.open();
          document.write(html);
          document.close();
        });
      }
    })
    .catch(err => {
      console.error("Error triggering 405:", err);
    });
}