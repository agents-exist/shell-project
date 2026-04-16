# Workflow

## How This Project Runs

This project is **issue-driven**. Everything flows through GitHub Issues.

```
┌─────────────────────────────────────────────┐
│                                             │
│   Create Issue → Assign → Work → PR/Commit  │
│        ↑                          │         │
│        └──── New issues spawned ◄─┘         │
│                                             │
└─────────────────────────────────────────────┘
```

### The Loop

1. **Issues are the backlog.** All tasks, research, ideas, and bugs live as GitHub Issues.
2. **Assign when possible.** If you know who should do it, assign them. Unassigned issues are fair game — whoever has bandwidth picks it up.
3. **Agents are woken by cron.** Bocchi and Kagura have scheduled jobs that check for open issues assigned to them (or unassigned). When there's work, they do it.
4. **Output goes into the repo.** Research docs, code, designs, notes — everything produced goes into the repo via PRs or direct commits.
5. **Work spawns more work.** Finishing one issue often reveals the next step. Open new issues for follow-ups before closing the current one.

### Issue Labels

| Label | Meaning |
|-------|---------|
| `research` | Investigation / survey / comparison |
| `design` | Architecture, protocol, or spec work |
| `code` | Implementation |
| `hardware` | Physical components, wiring, assembly |
| `question` | Needs discussion before action |
| `good first issue` | Easy entry point |

### Who Does What

- **Kagura 🌸** — Software lead, firmware, protocols
- **Bocchi 🎸** — Software, research, AI integration
- **Luna** — Hardware, assembly, sponsorship
- **Sakana** — Hardware, assembly, sponsorship

Agents (Kagura & Bocchi) handle software, research, and documentation autonomously via cron.
Humans (Luna & Sakana) handle physical hardware work and provide direction.

### Conventions

- One issue = one task. Keep scope small.
- Reference related issues with `#N`.
- Close issues via PR when possible (`Closes #N`).
- If stuck, comment on the issue and label it `question`.
