workspace "<System name>" "<One-sentence description of the system>." {

    !identifiers hierarchical

    model {

        # ─── External actors (people) ────────────────────────────────────────
        user = person "User" "<Primary user role and goal>."
        admin = person "Admin" "<Admin role and goal>."

        # ─── External systems ────────────────────────────────────────────────
        identityProvider = softwareSystem "Identity Provider" "<e.g. Auth0 / Cognito / Entra ID>." "External"
        emailService     = softwareSystem "Email Service" "<e.g. SES / SendGrid>." "External"

        # ─── The system under design ─────────────────────────────────────────
        target = softwareSystem "<System name>" "<One-sentence description>." {

            web = container "Web App" "<Front-end framework, e.g. Next.js 15>." "TypeScript" "WebApp"
            api = container "API"     "<Back-end framework, e.g. FastAPI>." "Python"      "Service"
            db  = container "Datastore" "<Engine, e.g. PostgreSQL 16>." "PostgreSQL" "Datastore"
        }

        # ─── L1 (System Context) relations ───────────────────────────────────
        user -> target "Uses"
        admin -> target "Administers"

        target -> identityProvider "Authenticates users via" "OAuth2 / OIDC"
        target -> emailService     "Sends transactional email via" "HTTPS / API"

        # ─── L2 (Container) relations ────────────────────────────────────────
        target.web -> target.api "Calls" "HTTPS + JSON"
        target.api -> target.db  "Reads from / writes to" "TCP/5432"

        target.api -> identityProvider "Verifies tokens with" "OIDC"
        target.api -> emailService     "Sends email via" "HTTPS"
    }

    views {

        systemContext target "L1-SystemContext" "C4 Level 1 — System Context for <System name>." {
            include *
            autoLayout lr
        }

        container target "L2-Container" "C4 Level 2 — Containers inside <System name>." {
            include *
            autoLayout tb
        }

        # ─── Component-level views (L3) — uncomment when detailed design lands ─
        #
        # component target.api "L3-Component-API" "Components inside the API container." {
        #     include *
        # }

        styles {
            element "Person" {
                shape Person
                background "#08427b"
                color "#ffffff"
            }
            element "External" {
                background "#999999"
                color "#ffffff"
            }
            element "WebApp" {
                background "#1168bd"
                color "#ffffff"
            }
            element "Service" {
                background "#0d8b8b"
                color "#ffffff"
            }
            element "Datastore" {
                shape Cylinder
                background "#cccccc"
                color "#000000"
            }
        }

        theme default
    }
}
