#!/usr/bin/env bash
#
# Execute the wallet-module doc-tests end-to-end and regenerate their Markdown.
#
# There are two specs:
#   wallet-module-runtime.test.yaml — packages this module as an .lgx, installs
#       it with lgpm, and drives it through a headless logoscore daemon.
#   wallet-ui-app.test.yaml         — builds the logos-wallet-ui standalone app
#       against this module commit and drives its QML UI headlessly, capturing
#       screenshots into outputs/images/.
#
# The runner is the shared `doctest` CLI
# (https://github.com/logos-co/logos-doctest), invoked directly via its flake.
# Each spec runs into ./outputs/ via --output-dir (so ui_test screenshots land in
# outputs/images/ next to the generated .md); `doctest generate` renders the .md;
# `doctest clean` then strips build artifacts, keeping only the .md and images/.
#
# To run against a local logos-doctest checkout instead of the published flake,
# set DOCTEST, e.g.:  DOCTEST="nix run path:../../logos-doctest --" ./run.sh
#
set -euo pipefail

# Run from this doctests/ directory regardless of where the script is invoked from.
cd "$(dirname "$0")"

# The doctest CLI. Override by exporting DOCTEST (space-separated command).
read -r -a DOCTEST <<< "${DOCTEST:-nix run github:logos-co/logos-doctest --}"
OUTPUT_DIR="./outputs"

# Build the doc-tests against THIS repo's current commit rather than the latest
# published flake. Each spec pins `github:logos-co/logos-wallet-module{release}`
# to $COMMIT via --release-for, so the runtime spec packages exactly what's
# checked out here and the UI spec overrides its wallet_module input to the same
# commit. Override by exporting COMMIT (e.g. a tag), or set COMMIT="" to fall
# back to latest master.
#
# Note: nix fetches the commit from the GitHub remote, so $COMMIT must be pushed
# to logos-co/logos-wallet-module. A local-only / uncommitted HEAD won't resolve;
# export COMMIT="" (or push first) in that case.
COMMIT="${COMMIT-$(git rev-parse HEAD)}"
RELEASE_FOR=()
if [ -n "${COMMIT}" ]; then
  RELEASE_FOR=(--release-for "logos-wallet-module=${COMMIT}")
  echo "==> Pinning logos-wallet-module to ${COMMIT}"
else
  echo "==> COMMIT empty; building from latest logos-wallet-module master"
fi

echo "==> Clearing previous ${OUTPUT_DIR}/"
# A prior run copies module artifacts out of the read-only nix store, so the
# directories land read-only (r-x) too. `rm -rf` can't delete files inside a
# directory it can't write to, so restore write permission first.
if [ -e "${OUTPUT_DIR}" ]; then
  chmod -R u+w "${OUTPUT_DIR}" 2>/dev/null || true
fi
rm -rf "${OUTPUT_DIR}"
mkdir -p "${OUTPUT_DIR}"

# Run each spec into ./outputs/ separately. --output-dir is single-spec, but
# passing it once per spec makes the runner write each spec's ui_test
# screenshots into outputs/images/ (beside the generated .md). The runtime spec
# has no screenshots; the UI spec populates outputs/images/.
for spec in *.test.yaml; do
  name="$(basename "${spec%.test.yaml}")"
  echo "==> Running ${spec} into ${OUTPUT_DIR}/"
  # ${RELEASE_FOR[@]+...} guards the expansion so an empty array doesn't trip
  # `set -u` on older bash (e.g. macOS's stock 3.2).
  "${DOCTEST[@]}" run "${spec}" \
    --verbose \
    --continue-on-fail \
    ${RELEASE_FOR[@]+"${RELEASE_FOR[@]}"} \
    --output-dir "${OUTPUT_DIR}/"

  echo "==> Generating ${OUTPUT_DIR}/${name}.md"
  "${DOCTEST[@]}" generate "${spec}" \
    ${RELEASE_FOR[@]+"${RELEASE_FOR[@]}"} \
    -o "${OUTPUT_DIR}/${name}.md"
done

echo "==> Cleaning build artifacts from ${OUTPUT_DIR}/ (keeps .md and images/)"
"${DOCTEST[@]}" clean "${OUTPUT_DIR}" --verbose

echo "==> Done. Rendered docs and screenshots are in ${OUTPUT_DIR}/"
