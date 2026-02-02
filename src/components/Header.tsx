import { motion } from "framer-motion";
import { Dices } from "lucide-react";

export const Header = () => {
  return (
    <header className="border-b bg-card/50 backdrop-blur-sm sticky top-0 z-50">
      <div className="container mx-auto px-4 py-4">
        <motion.div
          initial={{ opacity: 0, y: -20 }}
          animate={{ opacity: 1, y: 0 }}
          className="flex items-center gap-3"
        >
          <div className="p-2 rounded-lg bg-primary/10">
            <Dices className="h-8 w-8 text-primary" />
          </div>
          <div>
            <h1 className="text-2xl font-bold bg-gradient-to-r from-primary to-primary/60 bg-clip-text text-transparent">
              Loterie Stats
            </h1>
            <p className="text-sm text-muted-foreground">
              Analyse des tirages FDJ
            </p>
          </div>
        </motion.div>
      </div>
    </header>
  );
};
