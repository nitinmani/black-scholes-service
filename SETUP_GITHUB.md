# Setting Up the Public GitHub Repository

This guide walks you through creating a new public GitHub repository with only the Black-Scholes calculation code.

## Step 1: Create a New Repository on GitHub

1. Go to [GitHub](https://github.com) and sign in
2. Click the **+** icon in the top-right → **New repository**
3. Set the repository name (e.g., `black-scholes-service`)
4. Add a description: "C++ Black-Scholes option pricing microservice"
5. Select **Public**
6. **Do NOT** initialize with README, .gitignore, or license (we already have these)
7. Click **Create repository**

## Step 2: Initialize Git and Push

From the `black-scholes-public` directory:

```bash
cd c:\Users\nitin\OneDrive\Documents\tsg\black-scholes-public

# Initialize git
git init

# Add all files
git add .

# Create initial commit
git commit -m "Initial commit: Black-Scholes option pricing service"

# Add your GitHub repo as remote (replace YOUR_USERNAME and REPO_NAME with your values)
git remote add origin https://github.com/YOUR_USERNAME/black-scholes-service.git

# Push to GitHub
git branch -M main
git push -u origin main
```

## Step 3: Verify

- Visit your repository on GitHub
- Confirm only Black-Scholes related code is present
- No references to termsheetgenie, simulation service, or other proprietary code

## Important Notes

- The `black-scholes-public` folder is **separate** from your main `termsheetgenie-simulation-service` project
- Your main project stays private; this is an independent public repo
- To sync future Black-Scholes changes: manually copy updated files from the main project to `black-scholes-public`, then commit and push
