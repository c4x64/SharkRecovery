# GitHub Setup Instructions

## 1. Create GitHub Repository

```bash
# Create repo on GitHub (web interface or via CLI)
gh repo create shark-recovery-f22 --public --description "Shark Recovery for Samsung Galaxy F22 (SM-E225F) with KernelSU"
```

## 2. Push to GitHub

```bash
cd /Users/prabhas/SM-E225F
git remote add origin https://github.com/YOUR_USERNAME/shark-recovery-f22.git
git push -u origin main
```

## 3. Configure GitHub Secrets (optional)

No secrets needed for basic builds. For releases:
- `GITHUB_TOKEN` is auto-generated

## 4. Enable Actions

1. Go to repository Settings
2. Actions → General
3. Enable "Allow all actions and reusable workflows"
4. Enable "Read and write permissions" for actions

## 5. First Build

Push to main branch, Actions will start automatically.

## Repo Structure

```
shark-recovery-f22/
├── device/samsung/f22/     # Device tree
├── KernelSU/               # KernelSU integration
├── prebuilt/logos/         # Shark logo
├── .github/workflows/      # CI/CD
└── README.md
```

## Branch Strategy

- `main` - Stable releases
- `dev` - Development builds
- `ksu-experimental` - KernelSU testing
