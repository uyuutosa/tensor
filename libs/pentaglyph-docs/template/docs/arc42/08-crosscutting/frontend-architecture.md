---
status: Draft
owner: <placeholder>
last-reviewed: <YYYY-MM-DD>
---

# Frontend Software Architecture

> arc42 §8 Crosscutting. The discipline that keeps React / Next.js applications maintainable as features, async flows, and AI-driven surfaces accumulate.
> Authoritative section: <https://docs.arc42.org/section-8/>

This document is the design rule that every frontend building block in this project follows. It is not a tutorial on React, Next.js, TanStack Query, Zustand, Redux Toolkit, or Feature-Sliced Design — those are owned by their respective upstreams (linked at the bottom). The rule here is the *combination*: which library handles which kind of state, what belongs in a component, what does not, and how the layers depend on each other so that change stays local.

The thesis in one sentence: **separate the kinds of state, separate the responsibilities, evict business rules from UI components, and confine async I/O to explicit structures**. Everything below elaborates that sentence.

---

## 1. Why frontends need architecture now

Modern frontends are no longer a thin shell over the backend. React, Next.js, Vue, and Svelte applications routinely own asynchronous orchestration, cache, permission checks, form control, real-time streams, workflow state, and the run-state of AI agents. As soon as a frontend owns more than rendering, it inherits the same architectural pressure that backends always had — and the same failure modes when that pressure is ignored.

The default React tooling does not enforce any separation. `useState` and `useEffect` are general enough to absorb every responsibility into a single component: data fetching, derived state, async control, business rules, error handling, UI control. That works for a week. It collapses when the spec grows: payment-gating, optimistic updates, role-based visibility, multi-tab sync, server-sent events, retry-on-error. The component becomes a small application server, but without the discipline backends impose on application servers.

This document codifies the discipline. The recipe, stated up-front so the rest reads as elaboration:

```text
Next.js / React
+ Feature-based or Feature-Sliced architecture
+ TanStack Query for Server State
+ Zustand or Redux Toolkit for Client / Application State
+ React-official Effect minimisation (Effect is an escape hatch, not a data flow)
+ Lightweight DDD: domain model and policy as plain types and pure functions
```

Choosing libraries is not the architecture. Separating the kinds of state and the kinds of responsibility is.

---

## 2. Why frontends become complex

The single largest driver of frontend complexity is the *variety* of state, not its volume. An order-detail screen looks like one thing to the user, but on inspection contains:

```text
- Order data fetched from an API
- A modal open / closed flag that lives only in the browser
- Form draft input not yet submitted
- A business rule: is this order payable?
- An async status: is a request in flight?
- URL query parameters (search, page number)
- Permissions: can this user edit this order?
- Cross-component selection (e.g. which line item is highlighted)
```

These are not the same kind of thing. They have different lifetimes, different authorities, different consistency rules, and different test surfaces. Folding them all into `useState` and `useEffect` inside a single component flattens those distinctions and creates the canonical breakage pattern:

```tsx
function OrderPage() {
  const [order, setOrder] = useState(null)
  const [items, setItems] = useState([])
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState(null)
  const [selectedProduct, setSelectedProduct] = useState(null)
  const [totalAmount, setTotalAmount] = useState(0)

  useEffect(() => {
    setLoading(true)
    fetch("/api/orders/1")
      .then((res) => res.json())
      .then((data) => {
        setOrder(data)
        setItems(data.items)
        setLoading(false)
      })
      .catch((e) => {
        setError(e)
        setLoading(false)
      })
  }, [])

  useEffect(() => {
    setTotalAmount(
      items.reduce((sum, item) => sum + item.quantity * item.unitPrice, 0),
    )
  }, [items])

  async function addItem() {
    if (!selectedProduct) return
    setLoading(true)
    const res = await fetch("/api/orders/1/items", {
      method: "POST",
      body: JSON.stringify(selectedProduct),
    })
    const updatedOrder = await res.json()
    setOrder(updatedOrder)
    setItems(updatedOrder.items)
    setLoading(false)
  }

  return <div>...</div>
}
```

Display, network I/O, async status, derived state, business rules, and event handling all share one scope. It works the day it is written. Then the spec adds: paid orders cannot accept new items; cancelled orders hide the pay button; item adds need optimistic update with rollback on failure; out-of-stock items must be rejected; users without the edit permission must not see the controls; another tab updated this order and the local view must catch up; an SSE stream now reports the order's processing state in real time. Each addition wedges into the same `useState` / `useEffect` mass and the file becomes unowned.

The complexity is not "the screen is complex". It is "different kinds of state and responsibility live in the same place".

---

## 3. React's official position: Effect is not a general-purpose hook

React's own documentation classifies `useEffect` as an *escape hatch* for synchronising with systems outside React's data flow, and explicitly says Effects are unnecessary when no external system is involved. ([React][1]) The same docs warn against redundant or duplicated state — anything that can be computed from existing props or state during render should not be its own state. ([React][2])

This translates to a small number of concrete prohibitions that prevent most cargo-culted Effects.

A redundant-state pattern that should not exist:

```tsx
const [items, setItems] = useState<OrderItem[]>([])
const [totalAmount, setTotalAmount] = useState(0)

useEffect(() => {
  setTotalAmount(
    items.reduce((sum, item) => sum + item.quantity * item.unitPrice, 0),
  )
}, [items])
```

`totalAmount` is a derivation of `items`. It is not state. The correct form is just a computed value:

```tsx
const totalAmount = items.reduce(
  (sum, item) => sum + item.quantity * item.unitPrice,
  0,
)
```

If the computation is expensive, memoise it:

```tsx
const totalAmount = useMemo(
  () => items.reduce((sum, item) => sum + item.quantity * item.unitPrice, 0),
  [items],
)
```

The principle: Effects are for synchronising with browser APIs, third-party libraries, WebSocket / SSE, non-React widgets, the network — anything outside React. Effects are not for "when state A changes, update state B".

The rule, condensed:

```text
Derived values are not state.
Do not duplicate state.
Effects are for external synchronisation only.
Components are not workflow controllers.
```

---

## 4. Separate Server State from Client State

The single most consequential separation in modern frontend architecture is between **Server State** and **Client State**.

**Server State** is data whose source of truth lives on a server. The frontend fetches it, caches it, and renders it, but does not own it.

```text
- User profile
- Order list
- Product catalogue
- Inventory
- Notifications
- Dashboard metrics
- Approval requests
- AI agent run results
```

**Client State** is data the client owns: ephemeral UI flags, draft input, in-flight wizard step, dragging state. It has no canonical version on a server.

```text
- Modal open / closed
- Selected tab
- Wizard current step
- Unsaved form input
- Selected row id
- Temporary filter conditions
- In-progress drag state
```

TanStack Query's documentation explicitly positions itself as a Server State library and Redux / MobX / Zustand as Client State libraries — they are not substitutes for each other. ([TanStack Query][3])

The anti-pattern is to copy server data into a client store and again into local component state:

```text
API response → TanStack Query → Zustand → useState
```

Three copies, three sync paths, three places where they can drift. Don't.

The correct shape is: server data stays in TanStack Query (or a Server Component result); client UI flags live in `useState` / `useReducer` / Zustand / Redux Toolkit.

```tsx
const orderQuery = useQuery({
  queryKey: ["order", orderId],
  queryFn: () => orderApi.findById(orderId),
})
```

```tsx
const [isModalOpen, setModalOpen] = useState(false)
```

| State kind                | Example                              | Where it lives                                |
| ------------------------- | ------------------------------------ | --------------------------------------------- |
| Server State              | Orders, products, users from an API  | TanStack Query / Next.js Server Components    |
| Local UI state            | Modal open, selected tab             | `useState`                                    |
| Complex screen state      | Wizards, edit drafts                 | `useReducer` / Zustand                        |
| Application-wide state    | Permissions, complex transitions     | Zustand / Redux Toolkit                       |
| Derived state             | Totals, visibility flags             | Selector / pure function — not state          |
| URL state                 | Search, page number                  | URL search params                             |
| Business rules            | Can-pay, can-approve                 | `model` / `domain` layer                      |

The point is not "use Zustand" or "use Redux Toolkit". It is: do not put all state in a single store, and do not put server data in a client store at all.

---

## 5. The role of TanStack Query

TanStack Query is, in practice, the standard Server State manager in the React ecosystem. The official docs describe it as a library for asynchronous data fetching, caching, synchronisation, and server-state utilities — not a `fetch` replacement, but a manager for the long list of concerns that `fetch` does not address. ([TanStack Query][3])

What TanStack Query owns:

```text
- Loading state
- Error state
- Cache
- Stale detection
- Refetch on focus, reconnect, interval
- Background updates
- Mutations
- Cache update / invalidation after mutation
- Optimistic updates
- Pagination
- Infinite scroll
- Retry policy
```

Hand-rolling this in `useState` and `useEffect` produces the well-known fragile pattern:

```tsx
const [data, setData] = useState<Order | null>(null)
const [loading, setLoading] = useState(false)
const [error, setError] = useState<Error | null>(null)

useEffect(() => {
  let cancelled = false
  async function load() {
    setLoading(true)
    try {
      const res = await fetch(`/api/orders/${orderId}`)
      const json = await res.json()
      if (!cancelled) setData(json)
    } catch (e) {
      if (!cancelled) setError(e as Error)
    } finally {
      if (!cancelled) setLoading(false)
    }
  }
  load()
  return () => {
    cancelled = true
  }
}, [orderId])
```

That code, scattered across screens, never converges to a consistent error / retry / cache policy. The TanStack Query equivalent is one declaration:

```tsx
const orderQuery = useQuery({
  queryKey: ["order", orderId],
  queryFn: () => orderApi.findById(orderId),
})
```

Mutations follow the same shape:

```tsx
const queryClient = useQueryClient()

const payOrderMutation = useMutation({
  mutationFn: () => orderApi.pay(orderId),
  onSuccess: (updatedOrder) => {
    queryClient.setQueryData(["order", orderId], updatedOrder)
  },
})
```

What TanStack Query does *not* do is replace the application's client store. It is a server-state cache. Don't use it for modal flags or wizard steps; don't put server data into Zustand on top of it.

Operating rules:

```text
- API-sourced data lives in TanStack Query (or a Server Component result)
- Do not copy API data into Zustand
- After mutations, update or invalidate the query cache
- Do not hand-roll loading / error / refetch logic in useEffect
```

---

## 6. The roles of Zustand and Redux Toolkit

For Client State, Zustand and Redux Toolkit are the dominant choices and they trade off on different axes.

**Zustand** is described in its own docs as small, fast, and scalable: a hooks-based API with minimal boilerplate and minimal opinion. ([Zustand][4]) It fits when the state is shared across a few components and the transitions are not so complex that an explicit action log buys you anything.

```text
Use Zustand when:
- A handful of components share state (selected id, sidebar open, wizard step)
- Modal / drawer / panel coordination
- Edit drafts that don't justify a server round-trip yet
- AI Agent UI run-panel state, log-panel state
- Application state that is not large enough to warrant a Redux store
```

Example: an AI agent run panel.

```ts
import { create } from "zustand"

type AgentRunPanelState = {
  selectedRunId: string | null
  isLogPanelOpen: boolean
  isReviewPanelOpen: boolean
  selectRun: (runId: string) => void
  openLogPanel: () => void
  closeLogPanel: () => void
  openReviewPanel: () => void
  closeReviewPanel: () => void
}

export const useAgentRunPanelStore = create<AgentRunPanelState>((set) => ({
  selectedRunId: null,
  isLogPanelOpen: false,
  isReviewPanelOpen: false,

  selectRun: (runId) => set({ selectedRunId: runId }),
  openLogPanel: () => set({ isLogPanelOpen: true }),
  closeLogPanel: () => set({ isLogPanelOpen: false }),
  openReviewPanel: () => set({ isReviewPanelOpen: true }),
  closeReviewPanel: () => set({ isReviewPanelOpen: false }),
}))
```

**Redux Toolkit** is positioned by the Redux team as the standard way to write Redux today, designed to remove the original Redux boilerplate problem. ([Redux Toolkit][5]) It fits when transitions need to be auditable, replayable, or governed by a strict ledger.

```text
Use Redux Toolkit when:
- State transitions must be explicit and reviewable
- An action log / time-travel debugging is valuable
- A team needs to coordinate on shared state semantics
- Auditability or reproducibility is a requirement
- A large admin console with heavy workflow state
```

Selection heuristic:

```text
Small, fast, low boilerplate          → Zustand
Large, explicit, governed, auditable  → Redux Toolkit
Anything from an API                  → TanStack Query (neither of the above)
```

The shared rule is the one from §4: Zustand and Redux Toolkit hold *Client* / *Application* / *UI* state. They are not the place to cache server data.

---

## 7. Next.js App Router and Server Components: where the responsibility split moves

When the framework is Next.js, the App Router and React Server Components change where data fetching belongs. Next.js's documentation describes the App Router as a file-system router built around Server Components, Suspense, and Server Functions, and notes that Server Components can fetch data with `fetch`, an ORM, or a direct database client. ([Next.js][6])

The pre-App-Router default — fetch from the client in `useEffect` — is no longer the right default for first paint:

```tsx
"use client"

function OrderPage({ orderId }: { orderId: string }) {
  const [order, setOrder] = useState<Order | null>(null)

  useEffect(() => {
    fetch(`/api/orders/${orderId}`).then((r) => r.json()).then(setOrder)
  }, [orderId])

  return <div>...</div>
}
```

In an App Router project, the same screen fetches on the server:

```tsx
// app/orders/[orderId]/page.tsx
import { OrderDetail } from "./OrderDetail"

export default async function OrderPage({
  params,
}: {
  params: Promise<{ orderId: string }>
}) {
  const { orderId } = await params
  const order = await orderApi.findById(orderId)
  return <OrderDetail order={order} />
}
```

Mutation-driven UI — pay, approve, add item, start an agent — still belongs in Client Components and uses TanStack Query mutations:

```tsx
"use client"

export function PayOrderButton({ order }: { order: Order }) {
  const payOrder = usePayOrder(order.id)

  return (
    <button
      disabled={!canPay(order) || payOrder.isPending}
      onClick={() => payOrder.mutate()}
    >
      Pay
    </button>
  )
}
```

The responsibility split, as a table:

| Responsibility                          | Where it lives                            |
| --------------------------------------- | ----------------------------------------- |
| Initial-paint data                      | Server Component                          |
| User-driven mutation                    | Client Component + mutation hook          |
| Cache / refetch / stale handling        | TanStack Query                            |
| Screen-only UI state                    | `useState` / Zustand                      |
| Business rules                          | Domain / model layer                      |
| Database / secrets / privileged calls   | Server Component / Server Action / API    |

The principle: do not move everything to the client because "we already have a state library". Push to the server what the server can do; let the client own interaction.

---

## 8. Feature-Sliced Design and Feature-based architecture

Directory layout is not a cosmetic choice. It encodes the dependency graph and therefore the blast radius of every change. Feature-Sliced Design (FSD) describes layers as an organisational hierarchy ordered by responsibility scope and dependency direction, with the canonical layers `app`, `pages`, `widgets`, `features`, `entities`, `shared`. ([Feature-Sliced Design][7])

A worked layout:

```text
src/
  app/                       App-wide initialisation, providers, routing, global config
    providers/
    routes/
    layout.tsx

  pages/                     Page-level composition
    order-detail/
      ui/
      model/

  widgets/                   Composite UI built from features and entities
    order-summary/
      ui/
      model/

  features/                  User actions / use cases
    add-order-item/
      ui/
      model/
      api/

    pay-order/
      ui/
      model/
      api/

  entities/                  Domain concepts
    order/
      model/
      api/
      ui/

    customer/
      model/
      api/
      ui/

  shared/                    Generic UI, libs, API plumbing, config
    ui/
    api/
    lib/
    config/
```

Mapping FSD to a DDD-flavoured reading:

| FSD                    | DDD reading                              |
| ---------------------- | ---------------------------------------- |
| `entities/order`       | Order domain model                       |
| `features/pay-order`   | Pay-Order use case                       |
| `widgets/order-summary`| Composite display unit                   |
| `pages/order-detail`   | Page composition                         |
| `shared/api`           | Infrastructure (HTTP client, etc.)       |
| `app`                  | Composition root, providers              |

The structural rule is: organise by **business concept and feature**, not by **technical category**. The technical-category layout works when the project is small and breaks predictably as it grows.

Anti-pattern (technical-category):

```text
src/
  components/
    OrderPage.tsx
    OrderSummary.tsx
    PayButton.tsx
  hooks/
    useOrder.ts
    usePayOrder.ts
  api/
    orderApi.ts
  types/
    order.ts
  utils/
    orderUtils.ts
```

Everything related to "order" is scattered across five directories, and the dependency direction is implicit.

Preferred (feature / entity):

```text
src/
  entities/order/
    model/order.ts
    model/orderPolicy.ts
    api/orderApi.ts
    ui/OrderStatusBadge.tsx

  features/pay-order/
    model/usePayOrder.ts
    ui/PayOrderButton.tsx

  features/add-order-item/
    model/useAddOrderItem.ts
    ui/AddOrderItemButton.tsx

  widgets/order-summary/
    ui/OrderSummaryCard.tsx
```

Order-related changes are local. The dependency graph (`widgets` depend on `features` and `entities`; `features` depend on `entities` and `shared`; `entities` depend on `shared`) is enforceable via lint rules and obvious in code review.

---

## 9. Coexistence with Atomic Design

Feature-Sliced Design and Atomic Design are not in competition — they organise different axes. FSD organises **the application** by responsibility, dependency direction, and business meaning. Atomic Design organises **UI components** by visual granularity. They coexist cleanly when each is given the right scope.

### 9.1 Different axes

Atomic Design partitions UI by part-size:

```text
atoms       Button, Input, Label, Icon
molecules   SearchBox, FormField, UserAvatarWithName
organisms   Header, OrderCard, ReviewPanel
templates   OrderDetailTemplate
pages       OrderDetailPage
```

FSD partitions code by business meaning:

```text
shared, entities, features, widgets, pages, app
```

For an order app:

```text
entities/order
features/pay-order
features/add-order-item
widgets/order-summary
pages/order-detail
```

Atomic Design asks: *is this a Button, a FormField, or a Panel?* FSD asks: *is this an Order entity, a Pay use case, or an order-detail widget?* The same UI component can be classified differently along each axis, which is why neither subsumes the other.

### 9.2 Recommended coexistence: Atomic Design inside `shared/ui` only

The most workable arrangement places Atomic Design **inside `shared/ui` and nowhere else**:

```text
src/
  shared/
    ui/
      atoms/
        Button.tsx
        Input.tsx
        Icon.tsx
      molecules/
        FormField.tsx
        SearchBox.tsx
        ConfirmDialog.tsx
      organisms/
        AppHeader.tsx
        SideNavigation.tsx
```

Generic, business-agnostic UI parts get the Atomic-Design granularity treatment. Business-specific UI lives in `entities`, `features`, and `widgets`:

```text
src/
  entities/order/ui/
    OrderStatusBadge.tsx
    OrderAmount.tsx
  features/pay-order/ui/
    PayOrderButton.tsx
  widgets/order-summary/ui/
    OrderSummaryCard.tsx
```

The distinction matters. `Button` is genuinely an atom — a primitive, business-agnostic UI part. `PayOrderButton` *looks* like a button but *is* a payment use case in UI clothing. Filing it under `shared/ui/atoms/` would erase the business meaning and dilute the atom layer with hundreds of one-off feature buttons.

### 9.3 Atomic Design alone hides business boundaries

A pure Atomic-Design layout in a non-trivial app produces this:

```text
src/components/
  atoms/Button.tsx
  atoms/Input.tsx
  molecules/SearchBox.tsx
  molecules/OrderStatusBadge.tsx
  molecules/UserInfoRow.tsx
  organisms/OrderSummaryCard.tsx
  organisms/ReviewDecisionPanel.tsx
  organisms/AgentRunTimeline.tsx
  pages/OrderDetailPage.tsx
```

Granularity is organised, but order-related code fans out across `components/organisms/`, `components/molecules/`, `hooks/`, `api/`, `types/`, `utils/`. There is no single place where "the order domain" lives. Atomic Design has no opinion on business boundaries — it is a granularity vocabulary, not an application architecture.

### 9.4 FSD alone leaves `shared/ui` unstructured

Conversely, with FSD only and no Atomic-Design discipline in `shared/ui`, the bag tends to flatten:

```text
shared/ui/
  Button.tsx
  Input.tsx
  Modal.tsx
  FormField.tsx
  Header.tsx
  Table.tsx
  Badge.tsx
  Card.tsx
  Dropdown.tsx
```

Functional, but it stops scaling once the part count crosses ~20–30. Atomic Design's atoms / molecules / organisms layering reintroduces structure exactly where FSD has the least to say:

```text
shared/ui/
  atoms/
  molecules/
  organisms/
```

### 9.5 Atomic Design's weakness: granularity questions that don't matter

In a business app, debates like *"is `OrderStatusBadge` a molecule or an organism?"* and *"is `PayOrderButton` an atom or an organism?"* rarely have a productive answer. They are not the question to optimise for. The question that *does* matter is:

```text
Is this generic UI?
Is this Order-domain UI?
Is this Pay-Order feature UI?
Is this OrderDetail widget UI?
```

That is an FSD question, and it is the one with consequences for change isolation, ownership, and dependency direction.

### 9.6 Coexistence rule

State the rule once and apply it without negotiation:

```text
shared/ui                       → Atomic Design (atoms / molecules / organisms)
entities / features / widgets   → FSD only; do not force Atomic-Design classification
```

A combined directory layout:

```text
src/
  app/
    layout.tsx
    providers.tsx

  shared/
    ui/
      atoms/Button.tsx
      atoms/Input.tsx
      atoms/Spinner.tsx
      molecules/FormField.tsx
      molecules/ConfirmDialog.tsx
      molecules/EmptyState.tsx
      organisms/AppHeader.tsx
      organisms/SideNavigation.tsx
    api/httpClient.ts
    lib/date.ts
    lib/money.ts

  entities/
    order/
      model/order.ts
      model/orderPolicy.ts
      api/orderApi.ts
      ui/OrderStatusBadge.tsx
      ui/OrderAmount.tsx

  features/
    pay-order/
      model/usePayOrder.ts
      ui/PayOrderButton.tsx
    cancel-order/
      model/useCancelOrder.ts
      ui/CancelOrderButton.tsx

  widgets/
    order-summary/ui/OrderSummaryCard.tsx
    order-action-panel/ui/OrderActionPanel.tsx

  pages/
    order-detail/ui/OrderDetailPage.tsx
```

Atomic Design lives inside `shared/ui`; FSD owns everything outside it.

### 9.7 Summary

Both vocabularies pull their weight when they stay in their lane:

```text
Feature-Sliced Design   organises responsibility, dependency, business boundaries
Atomic Design           organises generic UI granularity
```

Concretely:

```text
Button is an atom.
FormField is a molecule.
AppHeader is an organism.

PayOrderButton is a feature.
OrderStatusBadge is entity UI.
OrderSummaryCard is a widget.
ReviewDecisionPanel is a widget.
AgentRunTimeline is a widget.
```

**Generic parts → Atomic Design. Business parts → Feature-Sliced Design.** That is the configuration that survives growth.

---

## 10. Lightweight DDD on the frontend

DDD is usually discussed in the backend context, where Aggregates, Repositories, and Application Services govern persistence boundaries. Importing that vocabulary verbatim into a frontend produces ceremony without payoff: frontends rarely own transactions or persistence boundaries. The useful sub-set is smaller and concrete.

The frontend-applicable parts:

```text
- Express domain concepts as explicit types
- Keep business rules out of React components
- Express "can / cannot" decisions as named functions
- Hide API I/O behind a thin layer
- Express user actions as use-case hooks
- Components do display and event handling, nothing more
```

Example — an order domain expressed as types and pure functions:

```ts
// entities/order/model/order.ts

export type OrderStatus = "draft" | "paid" | "cancelled"

export type OrderItem = {
  productId: string
  name: string
  quantity: number
  unitPrice: number
}

export type Order = {
  id: string
  status: OrderStatus
  items: OrderItem[]
}

export function totalAmount(order: Order): number {
  return order.items.reduce(
    (sum, item) => sum + item.quantity * item.unitPrice,
    0,
  )
}

export function canPay(order: Order): boolean {
  return order.status === "draft" && order.items.length > 0
}

export function canAddItem(order: Order): boolean {
  return order.status === "draft"
}

export function canCancel(order: Order): boolean {
  return order.status === "draft"
}
```

In the component, the rule has a name:

```tsx
// Anti-pattern: rule inlined in JSX
<button disabled={order.status !== "draft" || order.items.length === 0}>Pay</button>

// Preferred: rule named, referenced
<button disabled={!canPay(order)}>Pay</button>
```

The named version is testable in isolation, reusable across screens, and reads as specification:

```ts
describe("canPay", () => {
  it("returns true for a draft order with items", () => {
    const order: Order = {
      id: "order-1",
      status: "draft",
      items: [{ productId: "book-1", name: "DDD book", quantity: 1, unitPrice: 5000 }],
    }
    expect(canPay(order)).toBe(true)
  })

  it("returns false when there are no items", () => {
    expect(canPay({ id: "order-1", status: "draft", items: [] })).toBe(false)
  })
})
```

The whole point of frontend lightweight DDD is to **evict business judgement from UI components**. Components render and dispatch; rules live in the domain layer.

---

## 11. Why frontends prefer `type + pure function` over classes

Backend DDD models domain entities as classes that hold state and behaviour together:

```ts
class Order {
  constructor(
    readonly id: string,
    readonly status: OrderStatus,
    readonly items: OrderItem[],
  ) {}

  canPay(): boolean {
    return this.status === "draft" && this.items.length > 0
  }
}
```

That works on the backend because the entity is constructed once, lives in the application's memory, and is not serialised back and forth across cache layers. Frontends are different: data arrives from APIs as JSON, gets cached in TanStack Query, may be hydrated from the server via Server Components, and is regularly re-serialised. Plain objects survive that journey; class instances do not without a hydration step.

```json
{ "id": "order-1", "status": "draft", "items": [] }
```

Calling `order.canPay()` on this requires reconstruction:

```ts
const orderJson = await orderApi.findById(orderId)
const order = Order.fromJson(orderJson)
order.canPay()
```

In most frontends, that ceremony is not worth the price. The plain-object equivalent is just as expressive:

```ts
export type Order = {
  id: string
  status: OrderStatus
  items: OrderItem[]
}

export function canPay(order: Order): boolean {
  return order.status === "draft" && order.items.length > 0
}
```

This is not "abandoning DDD". It is adapting the model to the data flow: the domain still has explicit concepts, the rules still have names, the tests still cover the rules — but the carrier is JSON-compatible, not class-based.

```text
Backend DDD:    Entity = class with state and behaviour
Frontend DDD:   Entity = JSON-compatible plain object
                Behaviour = pure function / policy function
```

Classes still earn their place in:

```text
- Repositories
- API clients
- Use cases
- Domain services
- Domain models with non-trivial state-machine transitions
```

For example, a use-case class:

```ts
export class PayOrderUseCase {
  constructor(private readonly orderRepository: OrderRepository) {}

  async execute(orderId: string): Promise<Order> {
    const order = await this.orderRepository.findById(orderId)
    if (!canPay(order)) throw new Error("Order is not payable")
    return this.orderRepository.pay(orderId)
  }
}
```

But where the use case meets React, prefer hooks: the integration with TanStack Query, Suspense, and the React tree is more natural in function form.

---

## 12. Frontend hexagonal architecture

The hexagonal pattern's job is to keep the application core independent of external technologies. On a frontend, the external technologies include:

```text
- HTTP / REST APIs
- GraphQL clients
- LocalStorage / SessionStorage / IndexedDB
- WebSocket / SSE
- Analytics SDKs
- Auth SDKs
- Feature-flag SDKs
- Browser APIs
```

If a UI component calls these directly, the component owns those dependencies. The anti-pattern:

```tsx
function PayOrderButton({ orderId }: { orderId: string }) {
  async function handleClick() {
    await fetch(`/api/orders/${orderId}/pay`, { method: "POST" })
  }
  return <button onClick={handleClick}>Pay</button>
}
```

The button now knows the endpoint, the HTTP method, the transport, and is coupled to all of them.

The rewrite hides the API in a repository / adapter:

```ts
// entities/order/api/orderApi.ts

export async function payOrder(orderId: string): Promise<Order> {
  const res = await fetch(`/api/orders/${orderId}/pay`, { method: "POST" })
  if (!res.ok) throw new Error("Payment failed")
  return res.json()
}
```

Wraps it in a use-case hook:

```ts
// features/pay-order/model/usePayOrder.ts

export function usePayOrder(orderId: string) {
  const queryClient = useQueryClient()

  return useMutation({
    mutationFn: () => payOrder(orderId),
    onSuccess: (updatedOrder) => {
      queryClient.setQueryData(["order", orderId], updatedOrder)
    },
  })
}
```

And the component becomes display + dispatch only:

```tsx
// features/pay-order/ui/PayOrderButton.tsx

export function PayOrderButton({ order }: { order: Order }) {
  const payOrder = usePayOrder(order.id)
  return (
    <button
      disabled={!canPay(order) || payOrder.isPending}
      onClick={() => payOrder.mutate()}
    >
      Pay
    </button>
  )
}
```

The dependency direction:

```text
UI Component
  ↓
Feature Hook / ViewModel
  ↓
Application Use Case
  ↓
Domain Model / Domain Policy
  ↓
Port / API Interface
  ↑
API Adapter
  ↓
Backend API
```

API contract changes ripple through the `api` and `model` layers; the UI does not move.

---

## 13. Practical directory layout

A pragmatic feature-based layout for React / Next.js:

```text
src/
  app/
    layout.tsx
    providers.tsx
    globals.css

  shared/
    api/
      httpClient.ts
    ui/
      Button.tsx
      Card.tsx
      Dialog.tsx
    lib/
      date.ts
      money.ts
    config/
      env.ts

  entities/
    order/
      model/
        order.ts
        orderPolicy.ts
      api/
        orderApi.ts
      ui/
        OrderStatusBadge.tsx

    customer/
      model/customer.ts
      api/customerApi.ts
      ui/CustomerName.tsx

  features/
    pay-order/
      model/usePayOrder.ts
      ui/PayOrderButton.tsx

    add-order-item/
      model/useAddOrderItem.ts
      ui/AddOrderItemButton.tsx

    cancel-order/
      model/useCancelOrder.ts
      ui/CancelOrderButton.tsx

  widgets/
    order-summary/ui/OrderSummaryCard.tsx
    order-action-panel/ui/OrderActionPanel.tsx

  pages/
    order-detail/ui/OrderDetailPage.tsx
```

In a Next.js App Router project, `app/` plays the routing role; the FSD `pages/` slot is either dropped or used as the host for page-level composition that `app/<route>/page.tsx` calls into:

```text
src/
  app/
    orders/[orderId]/page.tsx
  entities/order/
  features/pay-order/
  widgets/order-detail/
```

Page files stay thin:

```tsx
// app/orders/[orderId]/page.tsx
import { OrderDetailPage } from "@/widgets/order-detail/ui/OrderDetailPage"
import { orderApi } from "@/entities/order/api/orderApi"

export default async function Page({
  params,
}: {
  params: Promise<{ orderId: string }>
}) {
  const { orderId } = await params
  const order = await orderApi.findById(orderId)
  return <OrderDetailPage initialOrder={order} />
}
```

`app/` does routing and initial fetch; the application UI lives in `widgets`, `features`, and `entities`.

---

## 14. Application to AI Agent UI

Agent UIs raise the bar. They juggle async execution, streaming, intermediate logs, tool results, user approvals, cancel / resume / retry, multi-agent state, run history, review cards, human-intervention points, SSE / WebSocket transport, and optimistic UI updates. Stuffing those into a single component fails faster than CRUD does.

A serviceable structure:

```text
src/
  entities/
    agent-run/
      model/
        agentRun.ts
        agentRunPolicy.ts
        agentRunEvent.ts
      api/
        agentRunApi.ts
        agentRunStream.ts
      ui/
        AgentRunStatusBadge.tsx

    review/
      model/review.ts
      model/reviewPolicy.ts
      api/reviewApi.ts
      ui/ReviewStatusBadge.tsx

  features/
    start-agent-run/
      model/useStartAgentRun.ts
      ui/StartAgentRunButton.tsx

    cancel-agent-run/
      model/useCancelAgentRun.ts
      ui/CancelAgentRunButton.tsx

    approve-review/
      model/useApproveReview.ts
      ui/ApproveReviewButton.tsx

    request-revision/
      model/useRequestRevision.ts
      ui/RequestRevisionButton.tsx

  widgets/
    agent-run-timeline/ui/AgentRunTimeline.tsx
    review-decision-panel/ui/ReviewDecisionPanel.tsx
    agent-run-console/
      model/useAgentRunConsole.ts
      ui/AgentRunConsole.tsx
```

State-machine policy lives in `agentRunPolicy.ts`:

```ts
export type AgentRunStatus =
  | "idle"
  | "running"
  | "waiting_for_approval"
  | "completed"
  | "failed"
  | "cancelled"

export type AgentRun = {
  id: string
  status: AgentRunStatus
  startedAt: string
  completedAt?: string
}

export function canCancelAgentRun(run: AgentRun): boolean {
  return run.status === "running" || run.status === "waiting_for_approval"
}

export function canApproveAgentRun(run: AgentRun): boolean {
  return run.status === "waiting_for_approval"
}

export function isTerminalStatus(status: AgentRunStatus): boolean {
  return status === "completed" || status === "failed" || status === "cancelled"
}
```

The UI dispatches against named rules:

```tsx
export function CancelAgentRunButton({ run }: { run: AgentRun }) {
  const cancelRun = useCancelAgentRun(run.id)

  return (
    <button
      disabled={!canCancelAgentRun(run) || cancelRun.isPending}
      onClick={() => cancelRun.mutate()}
    >
      Cancel run
    </button>
  )
}
```

SSE subscription is one of the legitimate uses of `useEffect` — it synchronises with an external system, exactly the case React's docs reserve Effects for ([React][1]):

```ts
export function useAgentRunStream(runId: string) {
  const queryClient = useQueryClient()

  useEffect(() => {
    const eventSource = new EventSource(`/api/agent-runs/${runId}/events`)

    eventSource.onmessage = (event) => {
      const payload = JSON.parse(event.data)
      queryClient.setQueryData(["agent-run", runId], payload.run)
      queryClient.setQueryData(["agent-run-events", runId], payload.events)
    }

    return () => {
      eventSource.close()
    }
  }, [runId, queryClient])
}
```

The Effect is justified, scoped, and named.

---

## 15. When this architecture is worth applying

Not every screen needs the full structure. A read-only list with no interaction is fine with a Server Component or a single TanStack Query hook plus minimal component split:

```text
- Product list
- News feed
- Settings save form
```

The structure earns its keep when at least one of the following is true:

```text
- Business rules used on more than one screen
- Non-trivial state-machine transitions
- Multiple API calls per screen
- Real loading / error / refetch / stale handling required
- UI state and business state actively mix
- Logic that must be unit-tested
- Spec is expected to evolve
```

The trigger questions:

```text
1. Is a rule reused across screens?
2. Is the state machine non-trivial?
3. Are there multiple coordinated API calls?
4. Are there real loading / error / retry concerns?
5. Are UI state and business state likely to interleave?
6. Is there logic worth testing in isolation?
7. Is the spec going to keep changing?
```

A "yes" on any of those is the cue to introduce feature-based layout, TanStack Query, a domain layer, and a Zustand or Redux Toolkit store.

---

## 16. Practical rules

Compressed checklist for day-to-day work.

### 16.1 Components are allowed to contain

```text
- JSX
- Display branching
- Event reception
- Small local state
- Prop wiring
```

### 16.2 Components should not contain

```text
- Direct, complex API I/O
- Business rules
- Complex state transitions
- Multi-API integration
- Cache control after mutation
- Permission-decision logic
- Domain calculations
```

### 16.3 `useEffect` — when to use, when not to

OK:

```text
- WebSocket subscription
- SSE subscription
- Browser-API synchronisation
- Third-party widget synchronisation
- DOM event subscription
- Timer control
```

Not OK:

```text
- Putting a derived value into a separate state
- Replicating event handlers as Effects
- Scattering API fetch logic across components
- "When state A changes, also set state B" patterns
```

(React's docs define both halves of this list. ([React][1]))

### 16.4 Server State

```text
- Server data lives in TanStack Query or a Server Component result
- Do not copy server data into Zustand
- After mutations, update or invalidate the query cache
- Loading / error states come from the query
```

### 16.5 Client State

```text
- Component-local: useState
- Complex local: useReducer
- Cross-component: Zustand
- Large, governed, auditable: Redux Toolkit
```

### 16.6 Domain logic

Name the rules. Examples of names you should be writing:

```text
- canPay(order)
- canCancel(order)
- calculateTotal(order)
- canApprove(review, user)
```

These live in `model` / `domain` / `policy` files, not in components.

---

## 17. Conclusion

The substance of modern frontend architecture is not the choice of React, Next.js, TanStack Query, Zustand, or Redux Toolkit. It is **separation of state and responsibility**:

```text
- Server State and Client State are different things
- UI State and Domain State are different things
- Business rules do not belong in components
- API I/O does not belong in components
- Effects are reserved for external synchronisation
- Code is grouped by feature, not by technical category
- In Next.js, Server and Client Components have different responsibilities
```

The recipe, restated:

```text
Next.js App Router / React
+ Feature-Sliced or Feature-based architecture
+ TanStack Query for Server State
+ Zustand or Redux Toolkit for Client / Application State
+ React-official Effect minimisation
+ Lightweight DDD: domain model and policy as types and pure functions
```

If backend DDD protects business rules from databases and external systems, frontend architecture protects components from the mess of state, async, and rules. Components do display and event reception; API I/O lives in the API layer; async caching lives in TanStack Query; client state lives in Zustand or Redux Toolkit; business rules live in domain / model. Pages, widgets, and features compose the rest.

The end state is the smallest set of components that look like this:

```text
Component:    display + event reception
Feature:      one user-facing action
Entity / Domain: business concepts and rules
Query / API:  Server State and transport
Store:        Client / UI state
App:          providers, routing, composition root
```

Frontends need architecture, just like backends — but it is *not* backend DDD ported wholesale. It is a lightweight, practical discipline tuned to React, Next.js, Server Components, TanStack Query, Zustand, and Feature-Sliced Design.

---

## References

- [1] React — *You Might Not Need an Effect*. <https://react.dev/learn/you-might-not-need-an-effect>
- [2] React — *Choosing the State Structure*. <https://react.dev/learn/choosing-the-state-structure>
- [3] TanStack Query — *Does TanStack Query replace Redux, MobX or other global state managers?* <https://tanstack.com/query/v5/docs/framework/react/guides/does-this-replace-client-state>
- [4] Zustand — *Introduction*. <https://zustand.docs.pmnd.rs/>
- [5] Redux Toolkit — *Getting Started*. <https://redux-toolkit.js.org/introduction/getting-started>
- [6] Next.js — *Getting Started: Fetching Data*. <https://nextjs.org/docs/app/getting-started/fetching-data>
- [7] Feature-Sliced Design — *Layers*. <https://feature-sliced.design/docs/reference/layers>

[1]: https://react.dev/learn/you-might-not-need-an-effect
[2]: https://react.dev/learn/choosing-the-state-structure
[3]: https://tanstack.com/query/v5/docs/framework/react/guides/does-this-replace-client-state
[4]: https://zustand.docs.pmnd.rs/
[5]: https://redux-toolkit.js.org/introduction/getting-started
[6]: https://nextjs.org/docs/app/getting-started/fetching-data
[7]: https://feature-sliced.design/docs/reference/layers
