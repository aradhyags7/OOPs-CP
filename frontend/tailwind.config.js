/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        page: '#F7F9FC',
        sidebar: '#F1F5F9',
        card: '#FFFFFF',
        primary: {
          DEFAULT: '#1769E8',
          hover: '#1358C4',
          light: '#EBF3FE',
        },
        text: {
          primary: '#111C36',
          secondary: '#64748B',
          muted: '#94A3B8',
        },
        border: {
          subtle: '#E2E8F0',
          hover: '#CBD5E1',
        },
        allow: {
          DEFAULT: '#22B86A',
          bg: '#EAF8F0',
          text: '#17874C',
        },
        block: {
          DEFAULT: '#E55353',
          bg: '#FDECEC',
          text: '#B92D2D',
        },
        reject: {
          DEFAULT: '#F59E0B',
          bg: '#FEF7EA',
          text: '#B45309',
        },
      },
      fontFamily: {
        sans: ['Inter', '-apple-system', 'BlinkMacSystemFont', 'Segoe UI', 'Roboto', 'sans-serif'],
        mono: ['JetBrains Mono', 'Cascadia Code', 'Consolas', 'monospace'],
      },
      boxShadow: {
        card: '0 1px 3px 0 rgba(0, 0, 0, 0.04), 0 1px 2px -1px rgba(0, 0, 0, 0.04)',
        hover: '0 4px 6px -1px rgba(0, 0, 0, 0.06), 0 2px 4px -2px rgba(0, 0, 0, 0.06)',
      },
      borderRadius: {
        card: '8px',
      }
    },
  },
  plugins: [],
}
