# Project Guidelines

## Git & PRs
- Do NOT add `Co-Authored-By` lines to commits or PRs.
- When creating PRs, include screenshots of EVERY application screen rendered by the test suite. Use the headless LVGL simulator PPM->PNG pipeline to generate them.
- After every commit that changes UI code, regenerate screenshots and update them on the PR before pushing.
- After updating a PR, set a 10-minute cron to check CI results. If tests fail, note which ones failed and work on a fix. Repeat until all tests pass.
