#!/usr/bin/env bash
set -euo pipefail

npm run build -- --warnings
npm run test:ts
npm run format:check

