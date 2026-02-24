<?php
#!/usr/bin/env php
ini_set('display_errors', 1);
error_reporting(E_ALL);

// Read POST data (PHP füllt $_POST bei application/x-www-form-urlencoded automatisch)
$post = $_POST;
if (empty($post)) {
    $rawData = file_get_contents("php://input");
    parse_str($rawData, $post);
}

// items check (wie gehabt)
if (!isset($post["items"]) || trim($post["items"]) === "") {
    echo "No ingredients provided.";
    exit;
}

// --- NEW: read uploadId from query string (fallback to port-based) ---
$uploadId = null;
if (isset($_GET['uploadId']) && trim($_GET['uploadId']) !== '') {
    $uploadId = $_GET['uploadId'];
} else {
    // fallback: keep old behaviour
    $port = isset($_SERVER["SERVER_PORT"]) ? $_SERVER["SERVER_PORT"] : "8080";
    $uploadId = "0_0_" . $port; // oder deine alte default-namenskonvention
}

// simple validation: only digits + underscores (pattern: e.g. 0_0_8080)
if (!preg_match('/^[0-9]+_[0-9]+_[0-9]+$/', $uploadId)) {
    header('HTTP/1.1 400 Bad Request');
    echo "Invalid uploadId.";
    exit;
}

// Build path using uploadId
$baseUploads = __DIR__ . "/../uploads"; // relativ zu cgi-bin
$dir = $baseUploads . "/" . $uploadId;
$filePath = $dir . "/" . $uploadId . ".txt";

// Ensure directories exist
if (!is_dir($baseUploads)) mkdir($baseUploads, 0755, true);
if (!is_dir($dir)) mkdir($dir, 0755, true);

// Write items
$items = preg_split("/\r\n|\n|\r/", $post["items"]);
$added = 0;
foreach ($items as $item) {
    $item = trim($item);
    if ($item !== "") {
        // sanitize a bit
        $item = preg_replace('/[\x00-\x1F\x7F]/', '', $item);
        file_put_contents($filePath, $item . "\n", FILE_APPEND | LOCK_EX);
        $added++;
    }
}

echo "✅ Added $added items.";
?>
