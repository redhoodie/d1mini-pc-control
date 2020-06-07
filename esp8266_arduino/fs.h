#include <FS.h>

const char* fsName = "SPIFFS";
FS* fileSystem = &SPIFFS;
SPIFFSConfig fileSystemConfig = SPIFFSConfig();

static bool fsOK;
String unsupportedFiles = String();

File uploadFile;

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

///////////////////////////////
// Utils to return HTTP codes, and determine content-type

//void replyOK() {
//  server.send(200, FPSTR(TEXT_PLAIN), "");
//}
//
//void replyOKWithMsg(String msg) {
//  server.send(200, FPSTR(TEXT_PLAIN), msg);
//}
//
void replyNotFound(String msg) {
  server.send(404, FPSTR(TEXT_PLAIN), msg);
}
//
//void replyBadRequest(String msg) {
//  Serial.println(msg);
//  server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
//}

void replyServerError(String msg) {
  Serial.println(msg);
  server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

////////////////////////////////
// Request handlers

/*
   Return the FS type, status and size info
*/
//void handleStatus() {
//  Serial.println("handleStatus");
//  FSInfo fs_info;
//  String json;
//  json.reserve(128);
//
//  json = "{\"type\":\"";
//  json += fsName;
//  json += "\", \"isOk\":";
//  if (fsOK) {
//    fileSystem->info(fs_info);
//    json += F("\"true\", \"totalBytes\":\"");
//    json += fs_info.totalBytes;
//    json += F("\", \"usedBytes\":\"");
//    json += fs_info.usedBytes;
//    json += "\"";
//  } else {
//    json += "\"false\"";
//  }
//  json += F(",\"unsupportedFiles\":\"");
//  json += unsupportedFiles;
//  json += "\"}";
//
//  server.send(200, "application/json", json);
//}

/*
   Return the list of files in the directory specified by the "dir" query string parameter.
   Also demonstrates the use of chuncked responses.
*/
//void handleFileList() {
//  if (!fsOK) {
//    return replyServerError(FPSTR(FS_INIT_ERROR));
//  }
//
//  if (!server.hasArg("dir")) {
//    return replyBadRequest(F("DIR ARG MISSING"));
//  }
//
//  String path = server.arg("dir");
//  if (path != "/" && !fileSystem->exists(path)) {
//    return replyBadRequest("BAD PATH");
//  }
//
//  Serial.println(String("handleFileList: ") + path);
//  Dir dir = fileSystem->openDir(path);
//  path.clear();
//
//  // use HTTP/1.1 Chunked response to avoid building a huge temporary string
//  if (!server.chunkedResponseModeStart(200, "text/json")) {
//    server.send(505, F("text/html"), F("HTTP1.1 required"));
//    return;
//  }
//
//  // use the same string for every line
//  String output;
//  output.reserve(64);
//  while (dir.next()) {
//    if (output.length()) {
//      // send string from previous iteration
//      // as an HTTP chunk
//      server.sendContent(output);
//      output = ',';
//    } else {
//      output = '[';
//    }
//
//    output += "{\"type\":\"";
//    if (dir.isDirectory()) {
//      output += "dir";
//    } else {
//      output += F("file\",\"size\":\"");
//      output += dir.fileSize();
//    }
//
//    output += F("\",\"name\":\"");
//    // Always return names without leading "/"
//    if (dir.fileName()[0] == '/') {
//      output += &(dir.fileName()[1]);
//    } else {
//      output += dir.fileName();
//    }
//
//    output += "\"}";
//  }
//
//  // send last string
//  output += "]";
//  server.sendContent(output);
//  server.chunkedResponseFinalize();
//}

/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(String path) {
  Serial.println(String("handleFileRead: ") + path);
  if (!fsOK) {
    replyServerError(FPSTR(FS_INIT_ERROR));
    return true;
  }

  if (path.endsWith("/")) {
    path += "index.htm";
  }

  String contentType;
  if (server.hasArg("download")) {
    contentType = F("application/octet-stream");
  } else {
    contentType = mime::getContentType(path);
  }

  if (!fileSystem->exists(path)) {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (fileSystem->exists(path)) {
    File file = fileSystem->open(path, "r");
    if (server.streamFile(file, contentType) != file.size()) {
      Serial.println("Sent less data than expected!");
    }
    file.close();
    return true;
  }

  return false;
}

/*
   As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed,
   return the path of the closest parent still existing
*/
//String lastExistingParent(String path) {
//  while (!path.isEmpty() && !fileSystem->exists(path)) {
//    if (path.lastIndexOf('/') > 0) {
//      path = path.substring(0, path.lastIndexOf('/'));
//    } else {
//      path = String();  // No slash => the top folder does not exist
//    }
//  }
//  Serial.println(String("Last existing parent: ") + path);
//  return path;
//}


/*
   Handle the creation/rename of a new file
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Create file    | parent of created file
   Create folder  | parent of created folder
   Rename file    | parent of source file
   Move file      | parent of source file, or remaining ancestor
   Rename folder  | parent of source folder
   Move folder    | parent of source folder, or remaining ancestor
*/
//void handleFileCreate() {
//  if (!fsOK) {
//    return replyServerError(FPSTR(FS_INIT_ERROR));
//  }
//
//  String path = server.arg("path");
//  if (path.isEmpty()) {
//    return replyBadRequest(F("PATH ARG MISSING"));
//  }
//
//  if (path == "/") {
//    return replyBadRequest("BAD PATH");
//  }
//  if (fileSystem->exists(path)) {
//    return replyBadRequest(F("PATH FILE EXISTS"));
//  }
//
//  String src = server.arg("src");
//  if (src.isEmpty()) {
//    // No source specified: creation
//    Serial.println(String("handleFileCreate: ") + path);
//    if (path.endsWith("/")) {
//      // Create a folder
//      path.remove(path.length() - 1);
//      if (!fileSystem->mkdir(path)) {
//        return replyServerError(F("MKDIR FAILED"));
//      }
//    } else {
//      // Create a file
//      File file = fileSystem->open(path, "w");
//      if (file) {
//        file.write((const char *)0);
//        file.close();
//      } else {
//        return replyServerError(F("CREATE FAILED"));
//      }
//    }
//    if (path.lastIndexOf('/') > -1) {
//      path = path.substring(0, path.lastIndexOf('/'));
//    }
//    replyOKWithMsg(path);
//  } else {
//    // Source specified: rename
//    if (src == "/") {
//      return replyBadRequest("BAD SRC");
//    }
//    if (!fileSystem->exists(src)) {
//      return replyBadRequest(F("SRC FILE NOT FOUND"));
//    }
//
//    Serial.println(String("handleFileCreate: ") + path + " from " + src);
//
//    if (path.endsWith("/")) {
//      path.remove(path.length() - 1);
//    }
//    if (src.endsWith("/")) {
//      src.remove(src.length() - 1);
//    }
//    if (!fileSystem->rename(src, path)) {
//      return replyServerError(F("RENAME FAILED"));
//    }
//    replyOKWithMsg(lastExistingParent(src));
//  }
//}
//


/*
   Delete the file or folder designed by the given path.
   If it's a file, delete it.
   If it's a folder, delete all nested contents first then the folder itself
   IMPORTANT NOTE: using recursion is generally not recommended on embedded devices and can lead to crashes (stack overflow errors).
   This use is just for demonstration purpose, and FSBrowser might crash in case of deeply nested filesystems.
   Please don't do this on a production system.
*/
//void deleteRecursive(String path) {
//  File file = fileSystem->open(path, "r");
//  bool isDir = file.isDirectory();
//  file.close();
//
//  // If it's a plain file, delete it
//  if (!isDir) {
//    fileSystem->remove(path);
//    return;
//  }
//
//  // Otherwise delete its contents first
//  Dir dir = fileSystem->openDir(path);
//
//  while (dir.next()) {
//    deleteRecursive(path + '/' + dir.fileName());
//  }
//
//  // Then delete the folder itself
//  fileSystem->rmdir(path);
//}


/*
   Handle a file deletion request
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
//void handleFileDelete() {
//  if (!fsOK) {
//    return replyServerError(FPSTR(FS_INIT_ERROR));
//  }
//
//  String path = server.arg(0);
//  if (path.isEmpty() || path == "/") {
//    return replyBadRequest("BAD PATH");
//  }
//
//  Serial.println(String("handleFileDelete: ") + path);
//  if (!fileSystem->exists(path)) {
//    return replyNotFound(FPSTR(FILE_NOT_FOUND));
//  }
//  deleteRecursive(path);
//
//  replyOKWithMsg(lastExistingParent(path));
//}

/*
   Handle a file upload request
*/
//void handleFileUpload() {
//  if (!fsOK) {
//    return replyServerError(FPSTR(FS_INIT_ERROR));
//  }
//  if (server.uri() != "/edit") {
//    return;
//  }
//  HTTPUpload& upload = server.upload();
//  if (upload.status == UPLOAD_FILE_START) {
//    String filename = upload.filename;
//    // Make sure paths always start with "/"
//    if (!filename.startsWith("/")) {
//      filename = "/" + filename;
//    }
//    Serial.println(String("handleFileUpload Name: ") + filename);
//    uploadFile = fileSystem->open(filename, "w");
//    if (!uploadFile) {
//      return replyServerError(F("CREATE FAILED"));
//    }
//    Serial.println(String("Upload: START, filename: ") + filename);
//  } else if (upload.status == UPLOAD_FILE_WRITE) {
//    if (uploadFile) {
//      size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
//      if (bytesWritten != upload.currentSize) {
//        return replyServerError(F("WRITE FAILED"));
//      }
//    }
//    Serial.println(String("Upload: WRITE, Bytes: ") + upload.currentSize);
//  } else if (upload.status == UPLOAD_FILE_END) {
//    if (uploadFile) {
//      uploadFile.close();
//    }
//    Serial.println(String("Upload: END, Size: ") + upload.totalSize);
//  }
//}


/*
   The "Not Found" handler catches all URI not explicitely declared in code
   First try to find and return the requested file from the filesystem,
   and if it fails, return a 404 page with debug information
*/
void handleNotFound() {
  if (!fsOK) {
    return replyServerError(FPSTR(FS_INIT_ERROR));
  }

  String uri = ESP8266WebServer::urlDecode(server.uri()); // required to read paths with blanks

  if (handleFileRead(uri)) {
    return;
  }

  iotWebConf.handleNotFound();
}

/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/
void handleGetEdit() {
  if (handleFileRead(F("/edit/index.htm"))) {
    return;
  }

#ifdef INCLUDE_FALLBACK_INDEX_HTM
  server.sendHeader(F("Content-Encoding"), "gzip");
  server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
#else
  replyNotFound(FPSTR(FILE_NOT_FOUND));
#endif

}


void fs_setup(void) {
  ////////////////////////////////
  // FILESYSTEM INIT

  fileSystemConfig.setAutoFormat(true);
  fileSystem->setConfig(fileSystemConfig);
  fsOK = fileSystem->begin();
  Serial.println(fsOK ? F("Filesystem initialized.") : F("Filesystem init failed!"));

  ////////////////////////////////
  // WEB SERVER INIT

//  // Filesystem status
//  server.on("/status", HTTP_GET, handleStatus);
//
//  // List directory
//  server.on("/list", HTTP_GET, handleFileList);
//
//  // Load editor
//  server.on("/edit", HTTP_GET, handleGetEdit);
//
//  // Create file
//  server.on("/edit",  HTTP_PUT, handleFileCreate);
//
//  // Delete file
//  server.on("/edit",  HTTP_DELETE, handleFileDelete);
//
//  // Upload file
//  // - first callback is called after the request has ended with all parsed arguments
//  // - second callback handles file upload at that location
//  server.on("/edit",  HTTP_POST, replyOK, handleFileUpload);

  // Default handler for all URIs not defined above
  // Use it to read files from filesystem
  server.onNotFound(handleNotFound);
 
  // Start server
//  server.begin();
//  Serial.println("HTTP server started");
}


//void fs_loop(void) {
//  server.handleClient();
////  MDNS.update();
//}
