# Windows에서 WSL로 개발 환경 설정하기

## 1. WSL 설치 (관리자 권한 PowerShell)

```powershell
wsl --install
```

재부팅 후 Ubuntu가 자동으로 설치됩니다.

## 2. Ubuntu 초기 설정

WSL Ubuntu 터미널을 열고:

```bash
# 패키지 업데이트
sudo apt update && sudo apt upgrade -y

# 필수 도구 설치
sudo apt install -y build-essential cmake python3 git

# Emscripten 설치
sudo apt install -y emscripten
```

## 3. 프로젝트 접근

Windows 파일 시스템은 `/mnt/c/`에 마운트됩니다:

```bash
cd /mnt/c/dev/wasm_powder
```

## 4. 빌드 실행

```bash
chmod +x build.sh
./build.sh
```

## 5. 웹 서버 실행

```bash
cd web
python3 -m http.server 8000
```

브라우저에서 `http://localhost:8000` 접속

## 팁

### VS Code에서 WSL 사용
1. VS Code에 "WSL" 확장 설치
2. `Ctrl+Shift+P` → "WSL: Connect to WSL" 선택
3. WSL 환경에서 직접 코딩 가능

### 파일 편집
- Windows에서 파일 편집 → WSL에서 빌드 가능
- 또는 VS Code WSL 모드로 완전히 WSL 환경에서 작업

### 성능
- `/mnt/c/` 경로는 약간 느릴 수 있음
- 성능이 중요하다면 프로젝트를 WSL 홈(`~/wasm_powder`)으로 복사:
  ```bash
  cp -r /mnt/c/dev/wasm_powder ~/
  cd ~/wasm_powder
  ```
