### PAC Utils
Collection of Utilities to Handle Idea Factory PAC Files

---
##### Unpack
Usage: `unpack.exe <archive1> [archive2...]`

Unpacks one or more pac files 
* Archive name will be used as directory name

```
C:\GAME00000.pac[File1]
C:\GAME00000.pac[Dir1\File1]
->
C:\GAME00000\File1
C:\GAME00000\Dir1\File2
```

---
##### Pack
Usage: `pack.exe <directory1> [directory2...]`

Packs one or more directories into new pac files
* Directory name will be used as pac name
* Archive root will be the directory contents

```
C:\GAME00000\File1
C:\GAME00000\Dir1\File2
->
C:\GAME00000.pac[File1]
C:\GAME00000.pac[Dir1\File1]
```

---
##### Patch
Usage: `patch.exe <archive|directory1> [archive|directory2...]`

Patches one or more pac files
* Will replace **files present in the archive** with the ones in directory with the same name as the archive
* No new files will be placed in the archive

```
C:\GAME00000\File1
C:\GAME00000\File2 ;Will not get replaced, since it doesn't exist in the archive
C:\GAME00000\Dir1\File2
->
C:\GAME00000.pac[File1]
C:\GAME00000.pac[Dir1\File1]
```